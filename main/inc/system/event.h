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

enum system_event_group_bits {
    WIFI_READY_BIT  = BIT0,
    BLUFI_READY_BIT = BIT1
};

enum daemon_event_group_bits {
    NFC_DAEMON_READY_BIT          = BIT0,
    GUI_DAEMON_RELOAD_BIT         = BIT1,
    AUDIO_DAEMON_READY_BIT        = BIT2,

    NTP_DAEMON_READY_BIT          = BIT3,
    NTP_DAEMON_FINISH_BIT         = BIT4,

    WIFI_DAEMON_START_BIT         = BIT5,
    WIFI_DAEMON_GOT_IP_BIT        = BIT6,
    WIFI_DAEMON_DISCONNECTED_BIT  = BIT7,

    BLUFI_DAEMON_READY_BIT        = BIT8,
    BLUFI_DAEMON_FINISH_BIT       = BIT9,

    HTTP2_DAEMON_OTA_RUN_BIT      = BIT10,
    HTTP2_DAEMON_OTA_READY_BIT    = BIT11,
    HTTP2_DAEMON_OTA_FINISH_BIT   = BIT12,
    HTTP2_DAEMON_OTA_FAILED_BIT   = BIT13,

    HTTP2_DAEMON_TOKEN_READY_BIT  = BIT14,
    HTTP2_DAEMON_TOKEN_FINISH_BIT = BIT15,
    HTTP2_DAEMON_TOKEN_FAILED_BIT = BIT16
};

#include <stdint.h>

extern EventGroupHandle_t system_event_group;
extern EventGroupHandle_t daemon_event_group;

extern void event_init(void);

#endif /* INC_SYSTEM_EVENT_H_ */
