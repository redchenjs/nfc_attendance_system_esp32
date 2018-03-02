/*
 * wifi.h
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef DEVICE_WIFI_C_
#define DEVICE_WIFI_C_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t wifi0_event_group;

extern const int WIFI0_CONNECTED_BIT;

extern char wifi0_mac_str[13];

extern void wifi0_init(void);

#endif /* DEVICE_WIFI_C_ */
