/*
 * ssd1331.c
 *
 *  Created on: 2018-02-10 15:55
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "driver/spi_master.h"

#include "device/spi.h"
#include "driver/ssd1331.h"

#define SSD1331_GPIO_PIN_DC  23
#define SSD1331_GPIO_PIN_RST 14

spi_transaction_t spi1_trans[3];

void ssd1331_init_board(void)
{
    gpio_set_direction(SSD1331_GPIO_PIN_DC,  GPIO_MODE_OUTPUT);
    gpio_set_direction(SSD1331_GPIO_PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(SSD1331_GPIO_PIN_DC,  0);
    gpio_set_level(SSD1331_GPIO_PIN_RST, 0);

    memset(spi1_trans, 0, sizeof(spi1_trans));
}

void ssd1331_setpin_dc(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(SSD1331_GPIO_PIN_DC, dc);
}

void ssd1331_setpin_reset(uint8_t rst)
{
    gpio_set_level(SSD1331_GPIO_PIN_RST, rst);
}

void ssd1331_write_cmd(uint8_t cmd)
{
    spi1_trans[0].length = 8;
    spi1_trans[0].tx_buffer = &cmd;
    spi1_trans[0].user = (void*)0;

    spi_device_transmit(spi1, &spi1_trans[0]);
}

void ssd1331_write_data(uint8_t data)
{
    spi1_trans[0].length = 8;
    spi1_trans[0].tx_buffer = &data;
    spi1_trans[0].user = (void*)1;

    spi_device_transmit(spi1, &spi1_trans[0]);
}

void ssd1331_refresh_gram(uint8_t *gram)
{
    spi1_trans[0].length = 3*8;
    spi1_trans[0].tx_data[0] = 0x15;    // Set Column Address
    spi1_trans[0].tx_data[1] = 0x00;    // 0, startx
    spi1_trans[0].tx_data[2] = SSD1331_SCREEN_WIDTH - 1;    // 95, endx
    spi1_trans[0].user = (void*)0;
    spi1_trans[0].flags = SPI_TRANS_USE_TXDATA;

    spi1_trans[1].length = 3*8,
    spi1_trans[1].tx_data[0] = 0x75;    // Set Row Address
    spi1_trans[1].tx_data[1] = 0x00;    // 0, starty
    spi1_trans[1].tx_data[2] = SSD1331_SCREEN_HEIGHT - 1;   // 63, endy
    spi1_trans[1].user = (void*)0;
    spi1_trans[1].flags = SPI_TRANS_USE_TXDATA;

    spi1_trans[2].length = SSD1331_SCREEN_WIDTH*SSD1331_SCREEN_HEIGHT*2*8;
    spi1_trans[2].tx_buffer = gram;
    spi1_trans[2].user = (void*)1;

    // Queue all transactions.
    for (int x=0; x<3; x++) {
        spi_device_queue_trans(spi1, &spi1_trans[x], portMAX_DELAY);
    }
}
