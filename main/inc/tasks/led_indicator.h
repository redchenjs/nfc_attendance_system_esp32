/*
 * led_indicator.h
 *
 *  Created on: 2018-02-13 15:43
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_LED_INDICATOR_H_
#define INC_LED_INDICATOR_H_

#include <stdint.h>

extern void led_indicator_set_mode(uint8_t led_level);
extern void led_indicator_task(void *pvParameter);

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern xTaskHandle led_indicator_task_handle;

#endif /* INC_LED_INDICATOR_H_ */
