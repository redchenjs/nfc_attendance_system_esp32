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
#include "user/key.h"
#include "user/nfc_app.h"
#include "user/audio_player.h"

#define SC_KEY_TAG "sc_key"

#ifdef CONFIG_ENABLE_SC_KEY
void sc_key_handle(void)
{
    key_set_scan_mode(KEY_SCAN_MODE_IDX_OFF);

    xEventGroupSetBits(wifi_event_group, WIFI_CFG_BIT);
    ESP_LOGI(SC_KEY_TAG, "start smartconfig");

    nfc_app_set_mode(NFC_APP_MODE_IDX_OFF);
#ifdef CONFIG_ENABLE_LED
    led_set_mode(LED_MODE_IDX_BLINK_F1);
#endif
#ifdef CONFIG_ENABLE_GUI
    gui_set_mode(GUI_MODE_IDX_GIF_CFG);
#endif
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    audio_player_play_file(MP3_FILE_IDX_WIFI_CFG);
#endif

    esp_wifi_disconnect();

    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    esp_smartconfig_start(&sc_cfg);
}
#endif
