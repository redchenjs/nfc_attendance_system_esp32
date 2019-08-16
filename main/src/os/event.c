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
#include "chip/wifi.h"
#include "user/nfc_app.h"
#include "user/ota.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/ntp.h"
#include "user/audio.h"

#define TAG "event"

EventGroupHandle_t os_event_group;
EventGroupHandle_t user_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            gui_show_image(0);
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, wifi_hostname));
            break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            EventBits_t uxBits = xEventGroupGetBits(os_event_group);
            if (!(uxBits & WIFI_CONFIG_BIT) && (uxBits & WIFI_READY_BIT)) {
                nfc_app_set_mode(0);
                led_set_mode(7);
                gui_show_image(0);
            }
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(os_event_group, WIFI_READY_BIT);
            break;
        }
        default:
            break;
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP: {
            xEventGroupSetBits(os_event_group, WIFI_READY_BIT);
            ntp_sync_time();
            ota_update();
            gui_show_image(3);
            led_set_mode(1);
            nfc_app_set_mode(1);
            break;
        }
        default:
            break;
    }
}

static void sc_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    static wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    switch (event_id) {
        case SC_EVENT_SCAN_DONE:
            ESP_LOGI(TAG, "scan done");
            break;
        case SC_EVENT_FOUND_CHANNEL:
            ESP_LOGI(TAG, "found channel");
            break;
        case SC_EVENT_GOT_SSID_PSWD:
            led_set_mode(5);
            ESP_LOGI(TAG, "got ssid and passwd");

            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            bzero(&wifi_config, sizeof(wifi_config_t));
            memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
            wifi_config.sta.bssid_set = evt->bssid_set;
            if (wifi_config.sta.bssid_set == true) {
                memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
            }
            ESP_LOGI(TAG, "ssid: %s", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "password: %s", wifi_config.sta.password);

            ESP_ERROR_CHECK(esp_wifi_disconnect());
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SC_EVENT_SEND_ACK_DONE:
            ESP_LOGI(TAG, "ack done");
            esp_smartconfig_stop();
            xEventGroupClearBits(os_event_group, WIFI_CONFIG_BIT);
            xEventGroupSetBits(user_event_group, KEY_SCAN_BIT);
            break;
        default:
            break;
    }
}

void event_init(void)
{
    os_event_group   = xEventGroupCreate();
    user_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL));
}
