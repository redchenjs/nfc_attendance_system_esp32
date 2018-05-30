/*
 * http2_client.c
 *
 *  Created on: 2018-02-24 19:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>
#include <netdb.h>

#include "http_parser.h"
#include "esp_log.h"

#include "user/http2_client.h"

#define TAG "http2_client"

#if defined(CONFIG_ENABLE_SERVER_CERT_VERIFY)
static const uint8_t *cert_file_ptr[][2] =  {
                                                {cert0_pem_ptr, cert0_pem_end}  // "DigiCert Global Root CA"
                                            };
static uint8_t cert_file_index = 0;
#endif

#define DBG_FRAME_SEND 1

/*
 * The implementation of nghttp2_send_callback type. Here we write
 * |data| with size |length| to the network and return the number of
 * bytes actually written. See the documentation of
 * nghttp2_send_callback for the details.
 */
static ssize_t nghttp2_callback_send(nghttp2_session *session, const uint8_t *data,
                                     size_t length, int flags, void *user_data)
{
    struct http2c_handle *hd = user_data;
    int rv = esp_tls_conn_write(hd->http2_tls, data, length);
    if (rv <= 0) {
        if (rv == MBEDTLS_ERR_SSL_WANT_READ || rv == MBEDTLS_ERR_SSL_WANT_WRITE) {
            rv = NGHTTP2_ERR_WOULDBLOCK;
        } else {
            rv = NGHTTP2_ERR_CALLBACK_FAILURE;
        }
    }
    return rv;
}

/*
 * The implementation of nghttp2_recv_callback type. Here we read data
 * from the network and write them in |buf|. The capacity of |buf| is
 * |length| bytes. Returns the number of bytes stored in |buf|. See
 * the documentation of nghttp2_recv_callback for the details.
 */
static ssize_t nghttp2_callback_recv(nghttp2_session *session, uint8_t *buf,
                                     size_t length, int flags, void *user_data)
{
    struct http2c_handle *hd = user_data;
    int rv = esp_tls_conn_read(hd->http2_tls, (char *)buf, (int)length);
    if (rv < 0) {
        if (rv == MBEDTLS_ERR_SSL_WANT_READ || rv == MBEDTLS_ERR_SSL_WANT_WRITE) {
            rv = NGHTTP2_ERR_WOULDBLOCK;
        } else {
            rv = NGHTTP2_ERR_CALLBACK_FAILURE;
        }
    } else if (rv == 0) {
        rv = NGHTTP2_ERR_EOF;
    }
    return rv;
}

static char *nghttp2_frame_type_str(int type)
{
    switch (type) {
    case NGHTTP2_HEADERS:
        return "HEADERS";
        break;
    case NGHTTP2_RST_STREAM:
        return "RST_STREAM";
        break;
    case NGHTTP2_GOAWAY:
        return "GOAWAY";
        break;
    case NGHTTP2_DATA:
        return "DATA";
        break;
    case NGHTTP2_SETTINGS:
        return "SETTINGS";
        break;
    case NGHTTP2_PUSH_PROMISE:
        return "PUSH_PROMISE";
        break;
    case NGHTTP2_PING:
        return "PING";
        break;
    default:
        return "other";
        break;
    }
}

static int nghttp2_callback_on_frame_send(nghttp2_session *session,
                                          const nghttp2_frame *frame, void *user_data)
{
    ESP_LOGD(TAG, "[on_frame_send] frame type %s", nghttp2_frame_type_str(frame->hd.type));
    switch (frame->hd.type) {
    case NGHTTP2_HEADERS:
        if (nghttp2_session_get_stream_user_data(session, frame->hd.stream_id)) {
            ESP_LOGD(TAG, "[frame-send] C ----------------------------> S (HEADERS)");
#if DBG_FRAME_SEND
            ESP_LOGD(TAG, "[frame-send] headers nv-len = %d", frame->headers.nvlen);
            const nghttp2_nv *nva = frame->headers.nva;
            size_t i;
            for (i = 0; i < frame->headers.nvlen; ++i) {
                ESP_LOGD(TAG, "[frame-send] %s : %s", nva[i].name, nva[i].value);
            }
#endif
        }
        break;
    }
    return 0;
}

static int nghttp2_callback_on_frame_recv(nghttp2_session *session,
                                          const nghttp2_frame *frame, void *user_data)
{
    ESP_LOGD(TAG, "[on_frame_recv][sid: %d] frame type %s", frame->hd.stream_id, nghttp2_frame_type_str(frame->hd.type));
    if (frame->hd.type != NGHTTP2_DATA) {
        return 0;
    }
    /* Subsequent processing only for data frame */
    http2c_frame_data_recv_cb_t data_recv_cb = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
    if (data_recv_cb) {
        struct http2c_handle *h2 = user_data;
        (*data_recv_cb)(h2, NULL, 0, DATA_RECV_FRAME_COMPLETE);
    }
    return 0;
}

static int nghttp2_callback_on_stream_close(nghttp2_session *session, int32_t stream_id,
                                            uint32_t error_code, void *user_data)
{

    ESP_LOGD(TAG, "[on_stream_close][sid %d]", stream_id);
    http2c_frame_data_recv_cb_t data_recv_cb = nghttp2_session_get_stream_user_data(session, stream_id);
    if (data_recv_cb) {
        struct http2c_handle *h2 = user_data;
        (*data_recv_cb)(h2, NULL, 0, DATA_RECV_RST_STREAM);
    }
    return 0;
}

static int nghttp2_callback_on_data_chunk_recv(nghttp2_session *session, uint8_t flags,
                                               int32_t stream_id, const uint8_t *data,
                                               size_t len, void *user_data)
{
    http2c_frame_data_recv_cb_t data_recv_cb;
    ESP_LOGD(TAG, "[on_data_chunk_recv][sid:%d]", stream_id);
    data_recv_cb = nghttp2_session_get_stream_user_data(session, stream_id);
    if (data_recv_cb) {
        ESP_LOGD(TAG, "[data-chunk] C <---------------------------- S (DATA chunk)"
                 "%lu bytes",
                 (unsigned long int)len);
        struct http2c_handle *h2 = user_data;
        (*data_recv_cb)(h2, (char *)data, len, 0);
        /* TODO: What to do with the return value: look for pause/abort */
    }
    return 0;
}

static int nghttp2_callback_on_header(nghttp2_session *session, const nghttp2_frame *frame,
                                      const uint8_t *name, size_t namelen, const uint8_t *value,
                                      size_t valuelen, uint8_t flags, void *user_data)
{
    ESP_LOGD(TAG, "[on_header][sid:%d] %s : %s", frame->hd.stream_id, name, value);
    return 0;
}

static int nghttp2_do_http2_connect(struct http2c_handle *hd)
{
    int ret;
    nghttp2_session_callbacks *callbacks;
    nghttp2_session_callbacks_new(&callbacks);
    nghttp2_session_callbacks_set_send_callback(callbacks, nghttp2_callback_send);
    nghttp2_session_callbacks_set_recv_callback(callbacks, nghttp2_callback_recv);
    nghttp2_session_callbacks_set_on_frame_send_callback(callbacks, nghttp2_callback_on_frame_send);
    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, nghttp2_callback_on_frame_recv);
    nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, nghttp2_callback_on_stream_close);
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, nghttp2_callback_on_data_chunk_recv);
    nghttp2_session_callbacks_set_on_header_callback(callbacks, nghttp2_callback_on_header);
    ret = nghttp2_session_client_new(&hd->http2_sess, callbacks, hd);
    if (ret != 0) {
        ESP_LOGE(TAG, "[do_http2_connect] new http2 session failed");
        nghttp2_session_callbacks_del(callbacks);
        return -1;
    }
    nghttp2_session_callbacks_del(callbacks);

    /* Create the SETTINGS frame */
    ret = nghttp2_submit_settings(hd->http2_sess, NGHTTP2_FLAG_NONE, NULL, 0);
    if (ret != 0) {
        ESP_LOGE(TAG, "[do_http2_connect] submit settings failed");
        return -1;
    }
    return 0;
}

int http2_client_connect(struct http2c_handle *hd, const char *uri)
{
    memset(hd, 0, sizeof(*hd));

    const char *proto[] = {"h2", NULL};
    esp_tls_cfg_t tls_cfg = {
        .alpn_protos = proto,
        .non_block = true,
#if defined(CONFIG_ENABLE_SERVER_CERT_VERIFY)
        .cacert_pem_buf = cert_file_ptr[cert_file_index][0],
        .cacert_pem_bytes = cert_file_ptr[cert_file_index][1] - cert_file_ptr[cert_file_index][0]
#endif
    };

    /* Hostname */
    struct http_parser_url u;
    http_parser_url_init(&u);
    http_parser_parse_url(uri, strlen(uri), 0, &u);
    hd->hostname = strndup(&uri[u.field_data[UF_HOST].off], u.field_data[UF_HOST].len);

    /* SSL/TLS Connection */
    if ((hd->http2_tls = esp_tls_conn_http_new(uri, &tls_cfg)) == NULL) {
        ESP_LOGE(TAG, "[connect] ssl/tls connection failed");
        goto error;
    }

    /* HTTP/2 Connection */
    if (nghttp2_do_http2_connect(hd) != 0) {
        ESP_LOGE(TAG, "[connect] http/2 connection failed with %s", uri);
        goto error;
    }

    return 0;
error:
    http2_client_free(hd);
    return -1;
}

void http2_client_free(struct http2c_handle *hd)
{
    if (hd->http2_sess) {
        nghttp2_session_del(hd->http2_sess);
        hd->http2_sess = NULL;
    }
    if (hd->http2_tls) {
        esp_tls_conn_delete(hd->http2_tls);
        hd->http2_tls = NULL;
    }
    if (hd->hostname) {
        free(hd->hostname);
        hd->hostname = NULL;
    }
}

int http2_client_execute(struct http2c_handle *hd)
{
    int ret;
    ret = nghttp2_session_send(hd->http2_sess);
    if (ret != 0) {
        ESP_LOGE(TAG, "[execute] http2 session send failed %d", ret);
        return -1;
    }
    ret = nghttp2_session_recv(hd->http2_sess);
    if (ret != 0) {
        ESP_LOGE(TAG, "[execute] http2 session recv failed %d", ret);
        return -1;
    }
    return 0;
}

int http2_client_do_get_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen, http2c_frame_data_recv_cb_t recv_cb)
{
    int ret = nghttp2_submit_request(hd->http2_sess, NULL, nva, nvlen, NULL, recv_cb);
    if (ret < 0) {
        ESP_LOGE(TAG, "[do_get_with_nv] headers call failed");
        return -1;
    }
    return ret;
}

int http2_client_do_get(struct http2c_handle *hd, const char *path, http2c_frame_data_recv_cb_t recv_cb)
{
    const nghttp2_nv nva[] = { HTTP2C_MAKE_NV(":method", "GET"),
                               HTTP2C_MAKE_NV(":scheme", "https"),
                               HTTP2C_MAKE_NV(":authority", hd->hostname),
                               HTTP2C_MAKE_NV(":path", path)
                             };
    return http2_client_do_get_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), recv_cb);
}

ssize_t http2_client_data_provider_cb(nghttp2_session *session, int32_t stream_id, uint8_t *buf,
                                      size_t length, uint32_t *data_flags,
                                      nghttp2_data_source *source, void *user_data)
{
    struct http2c_handle *h2 = user_data;
    http2c_putpost_data_cb_t data_cb = source->ptr;
    return (*data_cb)(h2, (char *)buf, length, data_flags);
}

int http2_client_do_putpost_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen,
                                    http2c_putpost_data_cb_t send_cb,
                                    http2c_frame_data_recv_cb_t recv_cb)
{

    nghttp2_data_provider http2c_data_provider;
    http2c_data_provider.read_callback = http2_client_data_provider_cb;
    http2c_data_provider.source.ptr = send_cb;
    int ret = nghttp2_submit_request(hd->http2_sess, NULL, nva, nvlen, &http2c_data_provider, recv_cb);
    if (ret < 0) {
        ESP_LOGE(TAG, "[do_putpost_with_nv] headers call failed");
        return -1;
    }
    return ret;
}

int http2_client_do_post(struct http2c_handle *hd, const char *path,
                         http2c_putpost_data_cb_t send_cb,
                         http2c_frame_data_recv_cb_t recv_cb)
{
    const nghttp2_nv nva[] = { HTTP2C_MAKE_NV(":method", "POST"),
                               HTTP2C_MAKE_NV(":scheme", "https"),
                               HTTP2C_MAKE_NV(":authority", hd->hostname),
                               HTTP2C_MAKE_NV(":path", path)
                             };
    return http2_client_do_putpost_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), send_cb, recv_cb);
}

int http2_client_do_put(struct http2c_handle *hd, const char *path,
                        http2c_putpost_data_cb_t send_cb,
                        http2c_frame_data_recv_cb_t recv_cb)
{
    const nghttp2_nv nva[] = { HTTP2C_MAKE_NV(":method", "PUT"),
                               HTTP2C_MAKE_NV(":scheme", "https"),
                               HTTP2C_MAKE_NV(":authority", hd->hostname),
                               HTTP2C_MAKE_NV(":path", path)
                             };
    return http2_client_do_putpost_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), send_cb, recv_cb);
}
