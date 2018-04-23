/*
 * pn532.c
 *
 *  Created on: 2018-04-23 13:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/gpio.h"
#include "driver/uart.h"

#define PN532_RST_PIN CONFIG_NFC_RST_PIN

void pn532_setpin_reset(uint8_t rst)
{
    gpio_set_level(PN532_RST_PIN, rst);
    uart_flush_input(1);
}

void pn532_init(void)
{
    gpio_set_direction(PN532_RST_PIN,  GPIO_MODE_OUTPUT);
    pn532_setpin_reset(1);
}
