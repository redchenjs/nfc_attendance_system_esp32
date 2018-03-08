/*
 * http2_client.h
 *
 *  Created on: 2018-02-24 19:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_HTTP2_CLIENT_H_
#define INC_TASKS_HTTP2_CLIENT_H_

#include "openssl/ssl.h"
#include "nghttp2/nghttp2.h"

struct http2c_handle {
    /* Ideally, CTX is per-program, so we could potentially take it out of this
     * per-connection structure
     */
    char             host[64];
    char             port[6];
    char             path[128];
    const uint8_t   *ca_file_ptr;
    uint32_t         ca_file_len;
    SSL_CTX         *ssl_ctx;      /*!< Pointer to the SSL context */
    SSL             *ssl;          /*!< Pointer to the SSL handle */
    nghttp2_session *http2_sess;   /*!< Pointer to the HTTP2 session handle */
    int              sockfd;       /*!< Socket file descriptor */
};

#define DATA_RECV_RST_STREAM      1
#define DATA_RECV_FRAME_COMPLETE  2

typedef int (*http2c_frame_data_recv_cb_t)(struct http2c_handle *handle, const char *data, size_t len, int flags);
typedef int (*http2c_putpost_data_cb_t)(struct http2c_handle *handle, char *data, size_t len, uint32_t *data_flags);

int  http2_client_connect(struct http2c_handle *hd, const char *uri);
void http2_client_free(struct http2c_handle *hd);

int http2_client_do_get(struct http2c_handle *hd, const char *path, http2c_frame_data_recv_cb_t recv_cb);
int http2_client_do_post(struct http2c_handle *hd, const char *path,
                   http2c_putpost_data_cb_t send_cb,
                   http2c_frame_data_recv_cb_t recv_cb);
int http2_client_do_put(struct http2c_handle *hd, const char *path,
                  http2c_putpost_data_cb_t send_cb,
                  http2c_frame_data_recv_cb_t recv_cb);

int http2_client_execute(struct http2c_handle *hd);

#define HTTP2C_MAKE_NV(NAME, VALUE)                                    \
  {                                                                    \
    (uint8_t *)NAME, (uint8_t *)VALUE, strlen(NAME), strlen(VALUE),    \
        NGHTTP2_NV_FLAG_NONE                                           \
  }

int http2_client_do_get_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen, http2c_frame_data_recv_cb_t recv_cb);
int http2_client_do_putpost_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen,
                              http2c_putpost_data_cb_t send_cb,
                              http2c_frame_data_recv_cb_t recv_cb);

#endif /* INC_TASKS_HTTP2_CLIENT_H_ */
