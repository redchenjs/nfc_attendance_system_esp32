/*
 * pn532.c
 *
 *  Created on: 2018-04-23 13:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/gpio.h"

void pn532_setpin_reset(uint8_t val)
{
    gpio_set_level(CONFIG_PN532_RST_PIN, val);
}

void pn532_init(void)
{
    gpio_set_direction(CONFIG_PN532_RST_PIN, GPIO_MODE_OUTPUT);

    pn532_setpin_reset(0);
}
