/*
 * event.h
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_SYSTEM_EVENT_H_
#define INC_SYSTEM_EVENT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum system_event_group_bits {
    WIFI_READY_BIT  = BIT0,
    WIFI_CONFIG_BIT = BIT1,
    INPUT_READY_BIT = BIT2
} system_event_group_bits_t;

typedef enum daemon_event_group_bits {
    NFC_DAEMON_READY_BIT          = BIT0,
    KEY_DAEMON_READY_BIT          = BIT1,
    GUI_DAEMON_RELOAD_BIT         = BIT2,
    AUDIO_DAEMON_READY_BIT        = BIT3,

    NTP_DAEMON_READY_BIT          = BIT4,
    NTP_DAEMON_FINISH_BIT         = BIT5,

    HTTP2_DAEMON_OTA_RUN_BIT      = BIT6,
    HTTP2_DAEMON_OTA_READY_BIT    = BIT7,
    HTTP2_DAEMON_OTA_FINISH_BIT   = BIT8,
    HTTP2_DAEMON_OTA_FAILED_BIT   = BIT9,

    HTTP2_DAEMON_TOKEN_READY_BIT  = BIT10,
    HTTP2_DAEMON_TOKEN_FINISH_BIT = BIT11,
    HTTP2_DAEMON_TOKEN_FAILED_BIT = BIT12
} daemon_event_group_bits_t;

#include <stdint.h>

extern EventGroupHandle_t system_event_group;
extern EventGroupHandle_t daemon_event_group;

extern void event_init(void);

#endif /* INC_SYSTEM_EVENT_H_ */
