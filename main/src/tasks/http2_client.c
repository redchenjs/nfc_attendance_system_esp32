/*
 * http2_client.c
 *
 *  Created on: 2018-02-24 19:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>
#include <netdb.h>

#include "esp_log.h"

#include "tasks/http2_client.h"

#define TAG "http2_client"

#define DBG_FRAME_SEND 1

static int http2_client_parse_uri(struct http2c_handle *hd, const char *uri)
{
    /* We only interested in https */
    size_t len, i, offset;
    size_t host_offset = 0;
    size_t port_offset = 0;
    size_t path_offset = 0;
    size_t host_len = 0;
    size_t port_len = 0;
    size_t path_len = 0;
    len = strlen(uri);
    if (len < 9 || memcmp("https://", uri, 8) != 0) {
        return -1;
    }
    host_offset = offset = 8;
    if (uri[offset] == '[') {
        /* IPv6 literal address */
        ++offset;
        ++host_offset;
        for (i = offset; i < len; ++i) {
            if (uri[i] == ']') {
                host_len = i - offset;
                offset = i + 1;
                break;
            }
        }
    } else {
        const char delims[] = ":/?#";
        for (i = offset; i < len; ++i) {
            if (strchr(delims, uri[i]) != NULL) {
                break;
            }
        }
        host_len = i - offset;
        offset = i;
    }
    if (host_len == 0) {
        return -1;
    }
    strncpy(hd->host, (char *)(uri + host_offset), host_len);
    hd->host[host_len] = 0;
    /* Assuming https */
    port_offset = offset;
    strncpy(hd->port, "443", 3);
    hd->port[3] = 0;
    if (offset < len) {
        if (uri[offset] == ':') {
            /* port */
            const char delims[] = "/?#";
            int port = 0;
            ++offset;
            ++port_offset;
            for (i = offset; i < len; ++i) {
                if (strchr(delims, uri[i]) != NULL) {
                    break;
                }
                if ('0' <= uri[i] && uri[i] <= '9') {
                    port *= 10;
                    port += uri[i] - '0';
                    if (port > 65535) {
                        return -1;
                    }
                } else {
                    return -1;
                }
            }
            if (port == 0) {
                return -1;
            }
            offset = i;
            port_len = (size_t)(offset - port_offset);
            strncpy(hd->port, (char *)(uri + port_offset), port_len);
            hd->port[port_len] = 0;
        }
    }
    path_offset = offset;
    for (i = offset; i < len; ++i) {
        if (uri[i] == '#') {
            break;
        }
    }
    if (i - offset == 0) {
        strncpy(hd->path, "/", 1);
        hd->path[1] = 0;
    } else {
        path_len = i - offset;
        strncpy(hd->path, (char *)(uri + path_offset), path_len);
        hd->path[path_len] = 0;
    }
    return 0;
}

int http2_client_connect_to_host(struct http2c_handle *hd)
{
    int ret;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;
    ret = getaddrinfo(hd->host, hd->port, &hints, &res);
    if (ret) {
        return -1;
    }

    ret = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (ret < 0) {
        goto err_freeaddr;
    }

    int fd = ret;
    ret = connect(fd, res->ai_addr, res->ai_addrlen);
    if (ret < 0) {
        goto err_freesocket;
    }

    hd->sockfd = fd;

    return 0;

err_freesocket:
    close(fd);
err_freeaddr:
    freeaddrinfo(res);
    return -1;
}

/* SSL connection on the TCP socket that is already connected */
static int http2_client_do_ssl_connect(struct http2c_handle *hd)
{
    SSL_CTX *ssl_ctx = SSL_CTX_new(TLSv1_2_client_method());
    if (!ssl_ctx) {
        return -1;
    }

    if (hd->ca_file_ptr != NULL && hd->ca_file_len != 0) {
        X509 *client_ca = d2i_X509(NULL, hd->ca_file_ptr, hd->ca_file_len);
        if (!client_ca) {
            return -1;
        }
        SSL_CTX_add_client_CA(ssl_ctx, client_ca);
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
    }

    unsigned char vector[] = "\x02h2";
    SSL_CTX_set_alpn_protos(ssl_ctx, vector, strlen((char *)vector));
    SSL *ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        SSL_CTX_free(ssl_ctx);
        return -1;
    }

    SSL_set_tlsext_host_name(ssl, hd->host);
    SSL_set_fd(ssl, hd->sockfd);
    int ret = SSL_connect(ssl);
    if (ret < 1) {
        int err = SSL_get_error(ssl, ret);
        ESP_LOGE(TAG, "[http2_client_do_ssl_connect] failed ssl handshake ret=%d error=%d", ret, err);
        SSL_CTX_free(ssl_ctx);
        SSL_free(ssl);
        return -1;
    }
    hd->ssl_ctx = ssl_ctx;
    hd->ssl = ssl;

    int flags = fcntl(hd->sockfd, F_GETFL, 0);
    fcntl(hd->sockfd, F_SETFL, flags | O_NONBLOCK);

    return 0;
}

/*
 * The implementation of nghttp2_send_callback type. Here we write
 * |data| with size |length| to the network and return the number of
 * bytes actually written. See the documentation of
 * nghttp2_send_callback for the details.
 */
static ssize_t nghttp2_callback_send_inner(struct http2c_handle *hd, const uint8_t *data,
                                   size_t length)
{
    int rv = SSL_write(hd->ssl, data, (int)length);
    if (rv <= 0) {
        int err = SSL_get_error(hd->ssl, rv);
        if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            rv = NGHTTP2_ERR_WOULDBLOCK;
        } else {
            rv = NGHTTP2_ERR_CALLBACK_FAILURE;
        }
    }
    return rv;
}

static ssize_t nghttp2_callback_send(nghttp2_session *session, const uint8_t *data,
                             size_t length, int flags, void *user_data)
{
    int rv = 0;
    struct http2c_handle *hd = user_data;

    int copy_offset = 0;
    int pending_data = length;

    /* Send data in 1000 byte chunks */
    while (copy_offset != (length - 1)) {
        int chunk_len = pending_data > 1000 ? 1000 : pending_data;
        int subrv = nghttp2_callback_send_inner(hd, data + copy_offset, chunk_len);
        if (subrv <= 0) {
            if (copy_offset) {
                /* If some data was xferred, send the number of bytes
                 * xferred */
                rv = copy_offset;
            } else {
                /* If not, send the error code */
                rv = subrv;
            }
            break;
        }
        copy_offset += chunk_len;
        pending_data -= chunk_len;
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
    int rv = SSL_read(hd->ssl, buf, (int)length);
    if (rv < 0) {
        int err = SSL_get_error(hd->ssl, rv);
        if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) {
            rv = NGHTTP2_ERR_WOULDBLOCK;
        } else {
            rv = NGHTTP2_ERR_CALLBACK_FAILURE;
        }
    } else if (rv == 0) {
        rv = NGHTTP2_ERR_EOF;
    }
    return rv;
}

char *nghttp2_frame_type_str(int type)
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
    ESP_LOGD(TAG, "[nghttp2_callback_on_frame_send] frame type %s", nghttp2_frame_type_str(frame->hd.type));
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
    ESP_LOGD(TAG, "[nghttp2_callback_on_frame_recv][sid: %d] frame type  %s", frame->hd.stream_id, nghttp2_frame_type_str(frame->hd.type));
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

    ESP_LOGD(TAG, "[nghttp2_callback_on_stream_close][sid %d]", stream_id);
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
    ESP_LOGD(TAG, "[nghttp2_callback_on_data_chunk_recv][sid:%d]", stream_id);
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
    ESP_LOGD(TAG, "[nghttp2_callback_on_header][sid:%d] %s : %s", frame->hd.stream_id, name, value);
    return 0;
}

static int http2_client_do_http2_connect(struct http2c_handle *hd)
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
        ESP_LOGE(TAG, "[http2_client_do_http2_connect] new http2 session failed");
        nghttp2_session_callbacks_del(callbacks);
        return -1;
    }
    nghttp2_session_callbacks_del(callbacks);

    /* Create the SETTINGS frame */
    ret = nghttp2_submit_settings(hd->http2_sess, NGHTTP2_FLAG_NONE, NULL, 0);
    if (ret != 0) {
        ESP_LOGE(TAG, "[http2_client_do_http2_connect] submit settings failed");
        return -1;
    }
    return 0;
}

int http2_client_connect(struct http2c_handle *hd, const char *uri)
{
    /* Parse the URI */
    if (http2_client_parse_uri(hd, uri) < 0) {
        ESP_LOGE(TAG, "[http2_client_connect] failed to parse URI");
        return -1;
    }

    /* TCP connection with the server */
    if (http2_client_connect_to_host(hd) < 0) {
        ESP_LOGE(TAG, "[http2_client_connect] failed to connect to %s", uri);
        return -1;
    }

    /* SSL Connection on the socket */
    if (http2_client_do_ssl_connect(hd) < 0) {
        ESP_LOGE(TAG, "[http2_client_connect] ssl handshake failed with %s", uri);
        goto error;
    }

    /* HTTP/2 Connection */
    if (http2_client_do_http2_connect(hd) < 0) {
        ESP_LOGE(TAG, "[http2_client_connect] http2 connection failed with %s", uri);
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
    if (hd->ssl) {
        SSL_free(hd->ssl);
        hd->ssl = NULL;
    }
    if (hd->ssl_ctx) {
        SSL_CTX_free(hd->ssl_ctx);
        hd->ssl_ctx = NULL;
    }
    if (hd->sockfd) {
        close(hd->sockfd);
        hd->ssl_ctx = 0;
    }
}

int http2_client_execute(struct http2c_handle *hd)
{
    int ret;
    ret = nghttp2_session_send(hd->http2_sess);
    if (ret != 0) {
        ESP_LOGE(TAG, "[http2_client_execute] http2 session send failed %d", ret);
        return -1;
    }
    ret = nghttp2_session_recv(hd->http2_sess);
    if (ret != 0) {
        ESP_LOGE(TAG, "[http2_client_execute] http2 session recv failed %d", ret);
        return -1;
    }
    return 0;
}

int http2_client_do_get_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen, http2c_frame_data_recv_cb_t recv_cb)
{
    int ret = nghttp2_submit_request(hd->http2_sess, NULL, nva, nvlen, NULL, recv_cb);
    if (ret < 0) {
        ESP_LOGE(TAG, "[http2_client_do_get_with_nv] headers call failed");
        return -1;
    }
    return ret;
}

int http2_client_do_get(struct http2c_handle *hd, const char *path, http2c_frame_data_recv_cb_t recv_cb)
{
    const nghttp2_nv nva[] = { HTTP2C_MAKE_NV(":method", "GET"),
                               HTTP2C_MAKE_NV(":scheme", "https"),
                               HTTP2C_MAKE_NV(":path", path),
                               HTTP2C_MAKE_NV(":authority", hd->host)
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
        ESP_LOGE(TAG, "[http2_client_do_putpost_with_nv] headers call failed");
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
                               HTTP2C_MAKE_NV(":path", path),
                               HTTP2C_MAKE_NV(":authority", hd->host)
                             };
    return http2_client_do_putpost_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), send_cb, recv_cb);
}

int http2_client_do_put(struct http2c_handle *hd, const char *path,
                  http2c_putpost_data_cb_t send_cb,
                  http2c_frame_data_recv_cb_t recv_cb)
{
    const nghttp2_nv nva[] = { HTTP2C_MAKE_NV(":method", "PUT"),
                               HTTP2C_MAKE_NV(":scheme", "https"),
                               HTTP2C_MAKE_NV(":path", path),
                               HTTP2C_MAKE_NV(":authority", hd->host)
                             };
    return http2_client_do_putpost_with_nv(hd, nva, sizeof(nva) / sizeof(nva[0]), send_cb, recv_cb);
}

