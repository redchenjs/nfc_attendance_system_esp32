/*
 * ntp_daemon.h
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_NTP_DAEMON_H_
#define INC_TASKS_NTP_DAEMON_H_

extern void ntp_daemon(void *pvParameter);
extern void ntp_sync_time(void);

#endif /* INC_TASKS_NTP_DAEMON_H_ */
