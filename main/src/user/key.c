/*
 * key.c
 *
 *  Created on: 2018-05-31 14:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"

#include "driver/gpio.h"

#include "user/gui.h"
#include "user/led.h"
#include "user/nfc.h"
#include "user/audio.h"
#include "system/event.h"
#include "user/smart_config.h"

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
    ESP_ERROR_CHECK(esp_smartconfig_start(smart_config_callback));
}

void key_daemon(void *pvParameter)
{
    uint8_t count[1] = {0};

    gpio_set_direction(CONFIG_SC_KEY_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_SC_KEY_PIN, GPIO_PULLUP_ONLY);

    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
    xEventGroupSetBits(daemon_event_group, KEY_DAEMON_READY_BIT);

    while (1) {
        xEventGroupWaitBits(
            daemon_event_group,
            KEY_DAEMON_READY_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        portTickType xLastWakeTime = xTaskGetTickCount();

        if (!gpio_get_level(CONFIG_SC_KEY_PIN)) {
            if (count[0]++ == 1) {
                count[0] = 0;
                key_smartconfig_handle();
            }
        }

        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_RATE_MS);
    }
}
