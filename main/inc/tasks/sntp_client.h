/*
 * sntp_client.h
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_SNTP_CLIENT_H_
#define INC_TASKS_SNTP_CLIENT_H_

#include <stdint.h>

enum sntp_client_status_table {
    SNTP_TIME_NOT_SET = 0,
    SNTP_TIME_SET     = 1
};

extern uint8_t sntp_client_status;

extern void sntp_client_task(void *pvParameter);

#endif /* INC_TASKS_SNTP_CLIENT_H_ */
