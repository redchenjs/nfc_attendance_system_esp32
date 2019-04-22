/*
 * spi.c
 *
 *  Created on: 2018-02-10 16:38
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/spi_master.h"

#include "driver/ssd1331.h"
#include "driver/st7735.h"

#define TAG "spi"

#ifdef CONFIG_ENABLE_GUI
spi_device_handle_t spi1;

void spi1_init(void)
{
    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = CONFIG_SPI_MOSI_PIN,
        .sclk_io_num = CONFIG_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
#ifdef CONFIG_SCREEN_PANEL_SSD1331
        .max_transfer_sz = SSD1331_SCREEN_WIDTH*SSD1331_SCREEN_HEIGHT*2
#else
        .max_transfer_sz = ST7735_SCREEN_WIDTH*ST7735_SCREEN_HEIGHT*2
#endif
    };
    spi_device_interface_config_t devcfg={
        .mode = 0,                                // SPI mode 0
        .spics_io_num = CONFIG_SPI_CS_PIN,        // CS pin
#ifdef CONFIG_SCREEN_PANEL_SSD1331
        .clock_speed_hz = 26000000,               // Clock out at 26 MHz
        .queue_size = 3,                          // We want to be able to queue 3 transactions at a time
        .pre_cb = ssd1331_setpin_dc,              // Specify pre-transfer callback to handle D/C line
#else
        .clock_speed_hz = 20000000,               // Clock out at 20 MHz
        .queue_size = 6,                          // We want to be able to queue 6 transactions at a time
        .pre_cb = st7735_setpin_dc,               // Specify pre-transfer callback to handle D/C line
#endif
        .flags = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &spi1));
    ESP_LOGI(TAG, "spi-1 initialized.");
}
#endif
