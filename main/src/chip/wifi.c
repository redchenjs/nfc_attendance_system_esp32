/*
 * wifi.c
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"

#define TAG "wifi"

static char wifi_hostname[40] = {0};
static char wifi_mac_string[18] = {0};
static uint8_t wifi_mac_address[6] = {0};

static wifi_config_t wifi_conf = {
    .sta = {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK
    }
};

wifi_config_t *wifi_get_conf(void)
{
    return &wifi_conf;
}

char *wifi_get_hostname(void)
{
    return wifi_hostname;
}

char *wifi_get_mac_string(void)
{
    return wifi_mac_string;
}

uint8_t *wifi_get_mac_address(void)
{
    return wifi_mac_address;
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_t *wifi_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_conf = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_conf));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));

#ifdef CONFIG_ENABLE_SC_KEY
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_conf));
#endif

    if (strlen((const char *)wifi_conf.sta.ssid) != 0) {
        ESP_LOGI(TAG, "found wifi configuration, ssid: %s", wifi_conf.sta.ssid);
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_conf));
    } else {
        ESP_LOGW(TAG, "no wifi configuration found");
    }

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, (uint8_t *)wifi_mac_address));
    snprintf(wifi_mac_string, sizeof(wifi_mac_string), MACSTR, MAC2STR(wifi_mac_address));

    snprintf(wifi_hostname, sizeof(wifi_hostname), "%s_%X%X%X", CONFIG_WIFI_HOSTNAME_PREFIX,
             wifi_mac_address[3], wifi_mac_address[4], wifi_mac_address[5]);
    ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_sta, wifi_hostname));

    ESP_LOGI(TAG, "initialized.");
}
