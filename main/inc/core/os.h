/*
 * os.h
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_CORE_OS_H_
#define INC_CORE_OS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum wifi_event_group_bits {
    WIFI_RDY_BIT = BIT0,
    WIFI_CFG_BIT = BIT1
} wifi_event_group_bits_t;

typedef enum user_event_group_bits {
    OS_PWR_DUMMY_BIT = 0x00,
    OS_PWR_RESET_BIT = BIT0,
    OS_PWR_SLEEP_BIT = BIT1,

    NTP_SYNC_RUN_BIT = BIT2,
    NTP_SYNC_SET_BIT = BIT3,

    NFC_APP_RUN_BIT  = BIT4,
    GUI_RLD_MODE_BIT = BIT5,

    KEY_SCAN_RUN_BIT = BIT6,
    KEY_SCAN_CLR_BIT = BIT7,

    AUDIO_PLAYER_RUN_BIT  = BIT8,
    AUDIO_PLAYER_IDLE_BIT = BIT9,

    HTTP_APP_OTA_RUN_BIT  = BIT10,
    HTTP_APP_OTA_DONE_BIT = BIT11,
    HTTP_APP_OTA_FAIL_BIT = BIT12,

    HTTP_APP_TOKEN_RUN_BIT  = BIT13,
    HTTP_APP_TOKEN_DONE_BIT = BIT14,
    HTTP_APP_TOKEN_FAIL_BIT = BIT15
} user_event_group_bits_t;

extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t user_event_group;

extern void os_pwr_reset_wait(EventBits_t bits);
extern void os_pwr_sleep_wait(EventBits_t bits);

extern void os_init(void);

#endif /* INC_CORE_OS_H_ */
