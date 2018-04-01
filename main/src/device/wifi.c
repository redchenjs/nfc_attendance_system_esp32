/*
 * wifi.c
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"

/*Set the SSID and Password via "make menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PASSWORD CONFIG_WIFI_PASSWORD
#define DEFAULT_HOSTNAME CONFIG_WIFI_HOSTNAME

char wifi0_mac_str[18] = {0};

#define TAG "wifi-0"

void wifi0_init(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_stored_config;
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_stored_config));
    if (strlen((char *)wifi_stored_config.sta.ssid) != 0) {
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_stored_config));
    } else if (strlen(CONFIG_WIFI_SSID) != 0) {
        strncpy(
            (char *)wifi_config.sta.ssid,
            (char *)CONFIG_WIFI_SSID,
            strlen(CONFIG_WIFI_SSID)
        );
        wifi_config.sta.ssid[strlen(CONFIG_WIFI_SSID)] = '\0';
        strncpy(
            (char *)wifi_config.sta.password,
            (char *)CONFIG_WIFI_PASSWORD,
            strlen(CONFIG_WIFI_PASSWORD)
        );
        wifi_config.sta.password[strlen(CONFIG_WIFI_PASSWORD)] = '\0';
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    }
    ESP_ERROR_CHECK(esp_wifi_start());
    uint8_t wifi0_mac[6] = {0};
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, wifi0_mac));
    snprintf(wifi0_mac_str, sizeof(wifi0_mac_str), MACSTR, MAC2STR(wifi0_mac));
    ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, CONFIG_WIFI_HOSTNAME));
}
