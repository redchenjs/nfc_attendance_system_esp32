/*
 * key.c
 *
 *  Created on: 2018-05-31 14:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "user/gui.h"
#include "user/led.h"
#include "user/nfc.h"
#include "user/audio.h"
#include "system/event.h"
#include "user/smartconfig.h"

#define TAG "key"

static void key_smartconfig_handle(void)
{
    EventBits_t uxBits = xEventGroupGetBits(system_event_group);
    if (uxBits & INPUT_READY_BIT) {
        ESP_LOGI(TAG, "smartconfig key pressed");
        xEventGroupClearBits(daemon_event_group, KEY_DAEMON_READY_BIT);
    } else {
        return;
    }

    xEventGroupSetBits(system_event_group, WIFI_CONFIG_BIT);

    nfc_set_mode(0);
    led_set_mode(7);
    gui_show_image(7);
    audio_play_file(7);

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    ESP_ERROR_CHECK(esp_smartconfig_start(smartconfig_callback));
}

void key_daemon(void *pvParameter)
{
    uint8_t count[1] = {0};

    gpio_set_direction(CONFIG_SC_KEY_PIN, GPIO_MODE_INPUT);

#if defined(CONFIG_SC_KEY_MODE_HIGH)
    gpio_pulldown_en(CONFIG_SC_KEY_PIN);
#else
    gpio_pullup_en(CONFIG_SC_KEY_PIN);
#endif

    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
    xEventGroupSetBits(daemon_event_group, KEY_DAEMON_READY_BIT);

    portTickType xLastWakeTime = xTaskGetTickCount();

    while (1) {
        xEventGroupWaitBits(
            daemon_event_group,
            KEY_DAEMON_READY_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

#if defined(CONFIG_SC_KEY_MODE_HIGH)
        if (gpio_get_level(CONFIG_SC_KEY_PIN)) {
#else
        if (!gpio_get_level(CONFIG_SC_KEY_PIN)) {
#endif
            if (count[0]++ == 1) {
                count[0] = 0;
                key_smartconfig_handle();
            }
        }

        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_RATE_MS);
    }
}
