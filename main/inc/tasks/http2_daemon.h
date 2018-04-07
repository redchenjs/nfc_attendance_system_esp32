/*
 * http2_daemon.h
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_HTTP2_DAEMON_H_
#define INC_TASKS_HTTP2_DAEMON_H_

#include <stdint.h>

// cert0.pem
extern const uint8_t cert0_pem_ptr[] asm("_binary_cert0_pem_start");
extern const uint8_t cert0_pem_end[] asm("_binary_cert0_pem_end");

extern void http2_daemon(void *pvParameter);

#endif /* INC_TASKS_HTTP2_DAEMON_H_ */
