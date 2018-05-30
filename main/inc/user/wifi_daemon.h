/*
 * wifi_daemon.h
 *
 *  Created on: 2018-04-01 15:04
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_WIFI_DAEMON_H_
#define INC_USER_WIFI_DAEMON_H_

#include <stdint.h>

extern void wifi_daemon(void *pvParameter);
extern void wifi_daemon(void *pvParameter);
extern void wifi_on_disconnected(void);
extern void wifi_on_got_ip(void);
extern void wifi_on_start(void);

#endif /* INC_USER_WIFI_DAEMON_H_ */
