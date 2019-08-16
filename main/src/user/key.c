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

#include "os/event.h"
#include "user/nfc_app.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio.h"

#define TAG "key"

#ifdef CONFIG_ENABLE_SMARTCONFIG
static void key_smartconfig_handle(void)
{
    EventBits_t uxBits = xEventGroupGetBits(os_event_group);
    if (uxBits & INPUT_READY_BIT) {
        ESP_LOGI(TAG, "smartconfig key pressed");
        xEventGroupClearBits(user_event_group, KEY_SCAN_BIT);
    } else {
        return;
    }

    xEventGroupSetBits(os_event_group, WIFI_CONFIG_BIT);

    nfc_app_set_mode(0);
    led_set_mode(7);
    gui_show_image(7);
    audio_play_file(7);

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
}
#endif

static void key_task_handle(void *pvParameter)
{
#ifdef CONFIG_ENABLE_SMARTCONFIG
    uint8_t count[1] = {0};
    portTickType xLastWakeTime;

    gpio_set_direction(CONFIG_SC_KEY_PIN, GPIO_MODE_INPUT);

#ifdef CONFIG_SC_KEY_MODE_HIGH
    gpio_pulldown_en(CONFIG_SC_KEY_PIN);
#else
    gpio_pullup_en(CONFIG_SC_KEY_PIN);
#endif

    xEventGroupSetBits(os_event_group, INPUT_READY_BIT);
    xEventGroupSetBits(user_event_group, KEY_SCAN_BIT);

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            KEY_SCAN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        xLastWakeTime = xTaskGetTickCount();

#ifdef CONFIG_SC_KEY_MODE_HIGH
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
#endif // CONFIG_ENABLE_SMARTCONFIG
}

void key_init(void)
{
   xTaskCreate(key_task_handle, "keyT", 2048, NULL, 5, NULL);
}
