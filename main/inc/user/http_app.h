/*
 * http_app.h
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP_APP_H_
#define INC_USER_HTTP_APP_H_

typedef enum {
    HTTP_REQ_CODE_DEV_VERIFY_TOKEN = 100, // 设备端请求口令验证
    HTTP_REQ_CODE_DEV_UPDATE_FW    = 101  // 设备端请求固件更新
} req_code_t;

// cert0.pem
extern const char cert0_pem_ptr[] asm("_binary_cert0_pem_start");
extern const char cert0_pem_end[] asm("_binary_cert0_pem_end");

extern void http_app_init(void);

#endif /* INC_USER_HTTP_APP_H_ */
