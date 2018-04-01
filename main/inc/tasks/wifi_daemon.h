/*
 * wifi_daemon.h
 *
 *  Created on: 2018-04-01 15:04
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_WIFI_DAEMON_H_
#define INC_TASKS_WIFI_DAEMON_H_

#include <stdint.h>

extern void wifi_daemon_reconnect(uint8_t mode);
extern void wifi_daemon_task(void *pvParameter);

#endif /* INC_TASKS_WIFI_DAEMON_H_ */
