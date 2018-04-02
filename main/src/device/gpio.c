/*
 * gpio.c
 *
 *  Created on: 2018-02-10 16:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/gpio.h"

#define LED1_PIN CONFIG_LED_PIN_NUM

void gpio0_init(void)
{
#if defined(CONFIG_ENABLE_LED)
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LED1_PIN,
        .pull_down_en = 0,
        .pull_up_en = 0
    };

    gpio_config(&io_conf);
#endif
}
