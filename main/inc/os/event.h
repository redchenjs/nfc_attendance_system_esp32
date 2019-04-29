/*
 * event.h
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_OS_EVENT_H_
#define INC_OS_EVENT_H_

#include "esp_event_loop.h"

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

    NFC_RUN_BIT    = BIT2,
    KEY_SCAN_BIT   = BIT3,
    AUDIO_RUN_BIT  = BIT4,
    GUI_RELOAD_BIT = BIT5,

    HTTP_OTA_RUN_BIT    = BIT6,
    HTTP_OTA_READY_BIT  = BIT7,
    HTTP_OTA_FAILED_BIT = BIT8,

    HTTP_TOKEN_RUN_BIT    = BIT9,
    HTTP_TOKEN_READY_BIT  = BIT10,
    HTTP_TOKEN_FAILED_BIT = BIT11
} user_event_group_bits_t;

extern EventGroupHandle_t os_event_group;
extern EventGroupHandle_t user_event_group;

extern esp_err_t os_event_handler(void *ctx, system_event_t *event);

extern void event_init(void);

#endif /* INC_OS_EVENT_H_ */
