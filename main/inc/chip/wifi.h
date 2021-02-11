/*
 * wifi.h
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_CHIP_WIFI_H_
#define INC_CHIP_WIFI_H_

#include <stdint.h>

#include "esp_wifi.h"

extern char *wifi_get_hostname(void);
extern char *wifi_get_mac_string(void);
extern uint8_t *wifi_get_mac_address(void);
extern wifi_config_t *wifi_get_config(void);

extern void wifi_init(void);

#endif /* INC_CHIP_WIFI_H_ */
