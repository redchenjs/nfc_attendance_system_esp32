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
    if (rst != 0) {
#if defined(CONFIG_PN532_IFCE_UART)
        uart_flush_input(NFC_UART_NUM);
#elif defined(CONFIG_PN532_IFCE_I2C)
        i2c_reset_tx_fifo(NFC_I2C_NUM);
        i2c_reset_rx_fifo(NFC_I2C_NUM);
#endif
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void pn532_init(void)
{
    gpio_set_direction(CONFIG_PN532_RST_PIN, GPIO_MODE_OUTPUT);
    pn532_setpin_reset(0);
}
