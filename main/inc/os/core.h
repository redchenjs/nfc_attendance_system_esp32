/*
 * core.h
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_OS_CORE_H_
#define INC_OS_CORE_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum os_event_group_bits {
    WIFI_READY_BIT  = BIT0,
    WIFI_CONFIG_BIT = BIT1,
    INPUT_READY_BIT = BIT2
} os_event_group_bits_t;

typedef enum user_event_group_bits {
    NTP_RUN_BIT   = BIT0,
    NTP_READY_BIT = BIT1,

    GUI_RELOAD_BIT     = BIT2,
    NFC_APP_RUN_BIT    = BIT3,
    KEY_SCAN_RUN_BIT   = BIT4,
    AUDIO_MP3_RUN_BIT  = BIT5,
    AUDIO_MP3_IDLE_BIT = BIT6,

    HTTP_OTA_RUN_BIT    = BIT7,
    HTTP_OTA_READY_BIT  = BIT8,
    HTTP_OTA_FAILED_BIT = BIT9,

    HTTP_TOKEN_RUN_BIT    = BIT10,
    HTTP_TOKEN_READY_BIT  = BIT11,
    HTTP_TOKEN_FAILED_BIT = BIT12
} user_event_group_bits_t;

extern EventGroupHandle_t os_event_group;
extern EventGroupHandle_t user_event_group;

extern void os_core_init(void);

#endif /* INC_OS_CORE_H_ */
