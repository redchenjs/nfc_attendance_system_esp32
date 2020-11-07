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

#include "core/os.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/nfc_app.h"
#include "user/audio_player.h"

#define SC_KEY_TAG "sc_key"

#ifdef CONFIG_ENABLE_SC_KEY
void sc_key_handle(void)
{
    xEventGroupClearBits(user_event_group, KEY_SCAN_RUN_BIT);

    ESP_LOGI(SC_KEY_TAG, "start smartconfig");
    xEventGroupSetBits(os_event_group, WIFI_CONFIG_BIT);

    nfc_app_set_mode(0);
    led_set_mode(5);
    gui_show_image(7);
    audio_player_play_file(7);

    esp_wifi_disconnect();

    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    esp_smartconfig_start(&sc_cfg);
}
#endif
