/*
 * pn532.c
 *
 *  Created on: 2018-04-23 13:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/gpio.h"
#include "driver/uart.h"

#define PN532_RST_PIN   CONFIG_PN532_RST_PIN

void pn532_setpin_reset(uint8_t rst)
{
    gpio_set_level(PN532_RST_PIN, rst);
    if (rst != 0) {
        uart_flush_input(1);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void pn532_init(void)
{
    gpio_set_direction(PN532_RST_PIN, GPIO_MODE_OUTPUT);
    pn532_setpin_reset(0);
}
