/*
 * os.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_smartconfig.h"

#include "freertos/event_groups.h"

#include "os/event.h"
#include "user/ota.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/nfc.h"
#include "user/ntp.h"
#include "user/audio.h"

#define TAG "event"

EventGroupHandle_t os_event_group;
EventGroupHandle_t user_event_group;

esp_err_t os_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            gui_show_image(0);
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP: {
            xEventGroupSetBits(os_event_group, WIFI_READY_BIT);
            ntp_sync_time();
            ota_update();
            gui_show_image(3);
            led_set_mode(1);
            nfc_set_mode(1);
            break;
        }
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED: {
            EventBits_t uxBits = xEventGroupGetBits(os_event_group);
            if (!(uxBits & WIFI_CONFIG_BIT) && (uxBits & WIFI_READY_BIT)) {
                nfc_set_mode(0);
                led_set_mode(7);
                gui_show_image(0);
            }
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(os_event_group, WIFI_READY_BIT);
            break;
        }
        case SYSTEM_EVENT_SCAN_DONE:
            break;
        default:
            break;
    }
    return ESP_OK;
}
