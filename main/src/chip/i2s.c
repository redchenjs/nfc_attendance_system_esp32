/*
 * i2s.c
 *
 *  Created on: 2018-02-10 16:38
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/i2s.h"

#define I2S0_TAG "i2s-0"
#define I2S1_TAG "i2s-1"

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
static i2s_config_t i2s_output_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .use_apll = 1,                                                          // use APLL
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL3,
    .tx_desc_auto_clear = true,                                             // auto clear tx descriptor on underflow
    .dma_buf_count = 8,
    .dma_buf_len = 128,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT                            // 2-channels
};
#endif

#if (CONFIG_AUDIO_OUTPUT_I2S_NUM == 0)
static void i2s0_init(void)
{
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_output_config, 0, NULL));

    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_I2S_BCLK_PIN,
        .ws_io_num = CONFIG_I2S_LRCK_PIN,
        .data_out_num = CONFIG_I2S_DOUT_PIN,
        .data_in_num = -1
    };
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config));

    ESP_LOGI(I2S0_TAG, "initialized, bck: %d, ws: %d, dout: %d, din: %d",
             pin_config.bck_io_num,
             pin_config.ws_io_num,
             pin_config.data_out_num,
             pin_config.data_in_num
    );
}

static void i2s0_deinit(void)
{
    ESP_ERROR_CHECK(i2s_driver_uninstall(I2S_NUM_0));

    ESP_LOGI(I2S0_TAG, "deinitialized.");
}
#endif

#if (CONFIG_AUDIO_OUTPUT_I2S_NUM == 1)
static void i2s1_init(void)
{
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &i2s_output_config, 0, NULL));

    i2s_pin_config_t pin_config = {
        .bck_io_num = CONFIG_I2S_BCLK_PIN,
        .ws_io_num = CONFIG_I2S_LRCK_PIN,
        .data_out_num = CONFIG_I2S_DOUT_PIN,
        .data_in_num = -1
    };
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &pin_config));

    ESP_LOGI(I2S1_TAG, "initialized, bck: %d, ws: %d, dout: %d, din: %d",
             pin_config.bck_io_num,
             pin_config.ws_io_num,
             pin_config.data_out_num,
             pin_config.data_in_num
    );
}

static void i2s1_deinit(void)
{
    ESP_ERROR_CHECK(i2s_driver_uninstall(I2S_NUM_1));

    ESP_LOGI(I2S1_TAG, "deinitialized.");
}
#endif

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
void i2s_output_init(void)
{
#if (CONFIG_AUDIO_OUTPUT_I2S_NUM == 0)
    i2s0_init();
#else
    i2s1_init();
#endif
}

void i2s_output_deinit(void)
{
#if (CONFIG_AUDIO_OUTPUT_I2S_NUM == 0)
    i2s0_deinit();
#else
    i2s1_deinit();
#endif
}

void i2s_output_set_sample_rate(int rate)
{
    if (rate != i2s_output_config.sample_rate) {
        i2s_output_config.sample_rate = rate;
        i2s_zero_dma_buffer(CONFIG_AUDIO_OUTPUT_I2S_NUM);
        i2s_set_sample_rates(CONFIG_AUDIO_OUTPUT_I2S_NUM, i2s_output_config.sample_rate);
    }
}
#endif
