#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
/*
 * led.c
 *
 *  Created on: 2017-6-4
 *      Author: redchenjs
 */

#define WIDORA_PIN_NUM_LED GPIO_NUM_25
#define LOCK_PIN_NUM       GPIO_NUM_2
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Main Program
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void led_on(void)
{
    rtc_gpio_set_level(WIDORA_PIN_NUM_LED, 1);
    rtc_gpio_set_level(LOCK_PIN_NUM, 1);
}

void led_off(void)
{
    rtc_gpio_set_level(WIDORA_PIN_NUM_LED, 0);
    rtc_gpio_set_level(LOCK_PIN_NUM, 0);
}

void led_init(void)
{
    rtc_gpio_init(WIDORA_PIN_NUM_LED);
    rtc_gpio_init(LOCK_PIN_NUM);
    
    rtc_gpio_set_direction(WIDORA_PIN_NUM_LED, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_set_direction(LOCK_PIN_NUM, RTC_GPIO_MODE_OUTPUT_ONLY);
}
