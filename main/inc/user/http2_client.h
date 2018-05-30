/*
 * http2_client.h
 *
 *  Created on: 2018-02-24 19:13
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP2_CLIENT_H_
#define INC_USER_HTTP2_CLIENT_H_

#include <stdint.h>

// cert0.pem
extern const uint8_t cert0_pem_ptr[] asm("_binary_cert0_pem_start");
extern const uint8_t cert0_pem_end[] asm("_binary_cert0_pem_end");

#include "esp_tls.h"
#include "nghttp2/nghttp2.h"

struct http2c_handle {
    nghttp2_session *http2_sess;   /*!< Pointer to the HTTP2 session handle */
    char            *hostname;     /*!< The hostname we are connected to */
    struct esp_tls  *http2_tls;    /*!< Pointer to the TLS session handle */
};

#define DATA_RECV_RST_STREAM      1
#define DATA_RECV_FRAME_COMPLETE  2

typedef int (*http2c_frame_data_recv_cb_t)(struct http2c_handle *handle, const char *data, size_t len, int flags);
typedef int (*http2c_putpost_data_cb_t)(struct http2c_handle *handle, char *data, size_t len, uint32_t *data_flags);

extern int  http2_client_connect(struct http2c_handle *hd, const char *uri);
extern void http2_client_free(struct http2c_handle *hd);

extern int http2_client_do_get(struct http2c_handle *hd, const char *path, http2c_frame_data_recv_cb_t recv_cb);
extern int http2_client_do_post(struct http2c_handle *hd, const char *path,
                                http2c_putpost_data_cb_t send_cb,
                                http2c_frame_data_recv_cb_t recv_cb);
extern int http2_client_do_put(struct http2c_handle *hd, const char *path,
                               http2c_putpost_data_cb_t send_cb,
                               http2c_frame_data_recv_cb_t recv_cb);

extern int http2_client_execute(struct http2c_handle *hd);

#define HTTP2C_MAKE_NV(NAME, VALUE)                                    \
  {                                                                    \
    (uint8_t *)NAME, (uint8_t *)VALUE, strlen(NAME), strlen(VALUE),    \
        NGHTTP2_NV_FLAG_NONE                                           \
  }

extern int http2_client_do_get_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen, http2c_frame_data_recv_cb_t recv_cb);
extern int http2_client_do_putpost_with_nv(struct http2c_handle *hd, const nghttp2_nv *nva, size_t nvlen,
                                           http2c_putpost_data_cb_t send_cb,
                                           http2c_frame_data_recv_cb_t recv_cb);

#endif /* INC_USER_HTTP2_CLIENT_H_ */
