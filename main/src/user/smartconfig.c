/*
 * smartconfig.c
 *
 *  Created on: 2018-04-01 15:04
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_smartconfig.h"

#include "user/led.h"
#include "system/event.h"

#define TAG "smartconfig"

void smartconfig_callback(smartconfig_status_t status, void *pdata)
{
    static wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "wait");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "finding channel");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            led_set_mode(5);
            ESP_LOGI(TAG, "getting ssid/passwd");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "link");

            memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
            memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
            wifi_config_t *config = pdata;
            strncpy((char *)wifi_config.sta.ssid, (char *)config->sta.ssid, strlen((char *)config->sta.ssid));
            strncpy((char *)wifi_config.sta.password, (char *)config->sta.password, strlen((char *)config->sta.password));
            ESP_LOGI(TAG, "ssid: %s", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "pswd: %s", wifi_config.sta.password);

            ESP_ERROR_CHECK(esp_wifi_disconnect());
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "link over");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = {0};
                memcpy(phone_ip, (uint8_t *)pdata, 4);
                ESP_LOGI(TAG, "phone ip is %d.%d.%d.%d", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            esp_smartconfig_stop();
            xEventGroupClearBits(system_event_group, WIFI_CONFIG_BIT);
            xEventGroupSetBits(daemon_event_group, KEY_DAEMON_READY_BIT);
            break;
        default:
            break;
    }
}
