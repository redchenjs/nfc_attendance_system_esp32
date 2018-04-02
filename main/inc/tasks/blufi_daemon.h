/*
 * blufi_daemon.h
 *
 *  Created on: 2018-03-31 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_BLUFI_DAEMON_H_
#define INC_TASKS_BLUFI_DAEMON_H_

#include <stdint.h>

extern void blufi_daemon_task(void *pvParameter);
extern void blufi_daemon_send_response(uint8_t response);

#endif /* INC_TASKS_BLUFI_DAEMON_H_ */
