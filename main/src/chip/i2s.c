/*
 * i2s.c
 *
 *  Created on: 2018-02-10 16:38
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"

#define I2S0_TAG "i2s-0"

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
static int i2s_output_sample_rate = 44100;
static int i2s_output_bits_per_sample = 16;

void i2s0_init(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .use_apll = chip_info.revision,                                         // Don't use apll on rev0 chips
        .sample_rate = i2s_output_sample_rate,
        .bits_per_sample = i2s_output_bits_per_sample,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           // 2-channels
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        .tx_desc_auto_clear = true                                              // Auto clear tx descriptor on underflow
    };
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
    i2s_pin_config_t pin_config = {
        .bck_io_num   = CONFIG_I2S_BCLK_PIN,
        .ws_io_num    = CONFIG_I2S_LRCK_PIN,
        .data_out_num = CONFIG_I2S_DOUT_PIN,
        .data_in_num  = -1                                                     // Not used
    };
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config));

    ESP_LOGI(I2S0_TAG, "initialized, bck: %d, ws: %d, dout: %d, din: %d",
             pin_config.bck_io_num,
             pin_config.ws_io_num,
             pin_config.data_out_num,
             pin_config.data_in_num
    );
}

void i2s_set_output_sample_rate(int rate)
{
    if (rate != i2s_output_sample_rate) {
        i2s_set_sample_rates(0, rate);
        i2s_output_sample_rate = rate;
    }
}

int i2s_get_output_sample_rate(void)
{
    return i2s_output_sample_rate;
}

int i2s_get_output_bits_per_sample(void)
{
    return i2s_output_bits_per_sample;
}
#endif
