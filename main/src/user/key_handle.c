/*
 * key_handle.c
 *
 *  Created on: 2019-07-06 10:35
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "os/core.h"
#include "user/nfc_app.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio_mp3.h"

#define SC_KEY_TAG "sc_key"

#ifdef CONFIG_ENABLE_SMARTCONFIG
void key_smartconfig_handle(void)
{
    EventBits_t uxBits = xEventGroupGetBits(os_event_group);
    if (uxBits & INPUT_READY_BIT) {
        ESP_LOGI(SC_KEY_TAG, "enter smartconfig mode");
        xEventGroupClearBits(user_event_group, KEY_SCAN_RUN_BIT);
    } else {
        return;
    }

    xEventGroupSetBits(os_event_group, WIFI_CONFIG_BIT);

    nfc_app_set_mode(0);
    led_set_mode(7);
    gui_show_image(7);
    audio_mp3_play(7);

    ESP_ERROR_CHECK(esp_wifi_disconnect());

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&sc_cfg));
}
#endif // CONFIG_ENABLE_SMARTCONFIG
