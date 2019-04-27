/*
 * init.c
 *
 *  Created on: 2018-02-10 16:37
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "os/event.h"
#include "chip/nvs.h"
#include "chip/spi.h"
#include "chip/i2s.h"
#include "chip/i2c.h"
#include "chip/wifi.h"
#include "chip/uart.h"
#include "board/pn532.h"
#include "user/gui.h"
#include "user/key.h"
#include "user/ntp.h"
#include "user/led.h"
#include "user/nfc.h"
#include "user/http.h"
#include "user/audio.h"

void os_init(void)
{
    os_event_group   = xEventGroupCreate();
    user_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(os_event_handler, NULL));
}

void chip_init(void)
{
    nvs_init();

    wifi_init();

#ifdef CONFIG_PN532_IFCE_UART
    uart1_init();
#else
    i2c0_init();
#endif

#ifdef CONFIG_ENABLE_AUDIO
    i2s0_init();
#endif

#ifdef CONFIG_ENABLE_GUI
    spi1_init();
#endif
}

void board_init(void)
{
    pn532_init();
}

void user_init(void)
{
    xTaskCreate(ntp_task, "ntp_task", 2048, NULL, 5, NULL);
    xTaskCreate(nfc_task, "nfc_task", 5120, NULL, 5, NULL);
    xTaskCreate(http_task, "http_task", 5120, NULL, 7, NULL);

#ifdef CONFIG_ENABLE_SMARTCONFIG
    xTaskCreate(key_task, "key_task", 2048, NULL, 5, NULL);
#endif

#ifdef CONFIG_ENABLE_LED
    xTaskCreate(led_task, "led_task", 1024, NULL, 6, NULL);
#endif

#ifdef CONFIG_ENABLE_GUI
    xTaskCreate(gui_task, "gui_task", 1024, NULL, 6, NULL);
#endif

#ifdef CONFIG_ENABLE_AUDIO
    xTaskCreate(audio_task, "audio_task", 8448, NULL, 7, NULL);
#endif
}
