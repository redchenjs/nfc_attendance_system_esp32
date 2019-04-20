/*
 * pn532.c
 *
 *  Created on: 2018-04-23 13:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "device/i2c.h"
#include "device/uart.h"

void pn532_setpin_reset(uint8_t rst)
{
    gpio_set_level(CONFIG_PN532_RST_PIN, rst);
}

void pn532_init(void)
{
    gpio_set_direction(CONFIG_PN532_RST_PIN, GPIO_MODE_OUTPUT);
    pn532_setpin_reset(0);
}
