/*
 * event.c
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

#include "system/event.h"
#include "user/ota_update.h"
#include "user/gui_daemon.h"
#include "user/led_daemon.h"
#include "user/nfc_daemon.h"
#include "user/ntp_daemon.h"
#include "user/audio_daemon.h"

#define TAG "event"

EventGroupHandle_t system_event_group;
EventGroupHandle_t daemon_event_group;

static esp_err_t system_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP: {
            xEventGroupSetBits(system_event_group, WIFI_READY_BIT);
            EventBits_t uxBits = xEventGroupGetBits(system_event_group);
            if (uxBits & WIFI_CONFIG_BIT) {
                esp_smartconfig_stop();
                xEventGroupClearBits(system_event_group, WIFI_CONFIG_BIT);
                xEventGroupSetBits(daemon_event_group, KEY_DAEMON_READY_BIT);
            }
            ntp_sync_time();
            ota_check_update();
            gui_show_image(3);
            led_set_mode(1);
            nfc_set_mode(1);
            break;
        }
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED: {
            xEventGroupClearBits(system_event_group, WIFI_READY_BIT);
            EventBits_t uxBits = xEventGroupGetBits(system_event_group);
            if (!(uxBits & WIFI_CONFIG_BIT)) {
                nfc_set_mode(0);
                led_set_mode(7);
                gui_show_image(0);
            }
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        }
        case SYSTEM_EVENT_SCAN_DONE:
            break;
        default:
            break;
    }
    return ESP_OK;
}

void event_init(void)
{
    system_event_group = xEventGroupCreate();
    daemon_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(system_event_handler, NULL));
}
