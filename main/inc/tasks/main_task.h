/*
 * main_task.h
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_MAIN_TASK_H_
#define INC_MAIN_TASK_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

enum system_event_group_bits {
    WIFI_READY_BIT = BIT0,
    SNTP_READY_BIT = BIT1
};

enum task_event_group_bits {
    TOKEN_VERIFIER_READY_BIT = BIT0,
    OLED_DISPLAY_RELOAD_BIT  = BIT1,
    NFC_INITIATOR_READY_BIT  = BIT2,
    MP3_PLAYER_READY_BIT     = BIT3
};

extern EventGroupHandle_t system_event_group;
extern EventGroupHandle_t task_event_group;

extern void main_task(void);

#endif /* INC_MAIN_TASK_H_ */
