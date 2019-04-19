/*
 * i2c.c
 *
 *  Created on: 2019-04-18 20:27
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "driver/i2c.h"

void i2c0_init(void)
{
    esp_err_t ret;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_PN532_TX_SDA_PIN,
        .scl_io_num = CONFIG_PN532_RX_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };
    ret = i2c_param_config(0, &conf);
    assert(ret == ESP_OK);
    ret = i2c_driver_install(0, conf.mode, 0, 0, 0);
    assert(ret == ESP_OK);
    ret = i2c_set_timeout(0, 80 * (I2C_APB_CLK_FREQ / conf.master.clk_speed));
    assert(ret == ESP_OK);
}
