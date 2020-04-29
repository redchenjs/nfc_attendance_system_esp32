/*
 * pn532.c
 *
 *  Created on: 2018-04-23 13:10
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/gpio.h"

#define TAG "pn532"

void pn532_setpin_reset(uint8_t val)
{
    gpio_set_level(CONFIG_PN532_RST_PIN, val);
}

void pn532_init(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(CONFIG_PN532_RST_PIN),
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "initialized, rst: %d", CONFIG_PN532_RST_PIN);
}
