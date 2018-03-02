/*
 * wifi.c
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include "freertos/event_groups.h"

/*Set the SSID and Password via "make menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PASS CONFIG_WIFI_PASSWORD

EventGroupHandle_t wifi0_event_group;

const int WIFI0_CONNECTED_BIT = BIT0;

char wifi0_mac_str[13] = {0};
uint8_t wifi0_mac[6] = {0};

#define TAG "wifi-0"

static char value_to_hex_char(const int value)
{
    char hex_char = '\0';
    if (value >= 0 && value <= 9) {
        hex_char = (char)(value + 0x30);
    } else if (value >= 10 && value <= 15) {
        hex_char = (char)(value - 10 + 0x61);
    }
    return hex_char;
}

static int str_to_hex_str(char *ori_str, char *hex_str)
{
    if (ori_str == NULL || hex_str == NULL) {
        return -1;
    }
    if (strlen(ori_str) == 0) {
        return -2;
    }
    while (*ori_str) {
        int tmp = (int)*ori_str;
        int high_byte = tmp >> 4;
        int low_byte  = tmp & 15;
        *hex_str++ = value_to_hex_char(high_byte);
        *hex_str++ = value_to_hex_char(low_byte);
        ori_str++;
    }
    *hex_str = '\0';
    return 0;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi0_event_group, WIFI0_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(wifi0_event_group, WIFI0_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi0_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    tcpip_adapter_init();
    wifi0_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PASS,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_LOGI(TAG, "setting wifi configuration, ssid: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, wifi0_mac));
    str_to_hex_str((char *)wifi0_mac, wifi0_mac_str);
    ESP_ERROR_CHECK(esp_wifi_start());
}
