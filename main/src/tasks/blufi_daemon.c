/*
 * blufi_daemon.c
 *
 *  Created on: 2018-03-31 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_blufi_api.h"

#include "device/bt.h"
#include "device/blufi.h"
#include "system/event.h"

#define TAG "blufi_daemon"

void blufi_daemon_send_response(uint8_t response)
{
#if defined(CONFIG_ENABLE_BLUFI)
    if (blufi0_status == BLUFI_CONNECTED) {
        switch (response) {
        case 0:
            xEventGroupSetBits(task_event_group, BLUFI_DAEMON_SEND_WIFI_LIST_BIT);
            break;
        case 1:
            xEventGroupSetBits(task_event_group, BLUFI_DAEMON_SEND_WIFI_REPORT_BIT);
            break;
        default:
            break;
        }
    }
#endif
}

void blufi_daemon_task(void *pvParameter)
{
    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(
            task_event_group,
            BLUFI_DAEMON_SEND_WIFI_LIST_BIT | BLUFI_DAEMON_SEND_WIFI_REPORT_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );
        if ((uxBits & BLUFI_DAEMON_SEND_WIFI_LIST_BIT) != 0) {
            uint16_t ap_count = 0;
            esp_wifi_scan_get_ap_num(&ap_count);
            if (ap_count == 0) {
                ESP_LOGW(TAG, "no ap found");
                break;
            }
            wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
            if (!ap_list) {
                ESP_LOGE(TAG, "malloc error, ap_list is NULL");
                break;
            }
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));
            esp_blufi_ap_record_t *blufi_ap_list = (esp_blufi_ap_record_t *)malloc(ap_count * sizeof(esp_blufi_ap_record_t));
            if (!blufi_ap_list) {
                if (ap_list) {
                    free(ap_list);
                }
                ESP_LOGE(TAG, "malloc error, blufi_ap_list is NULL");
                break;
            }
            for (int i=0; i<ap_count; i++) {
                blufi_ap_list[i].rssi = ap_list[i].rssi;
                memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
            }
            esp_blufi_send_wifi_list(ap_count, blufi_ap_list);
            esp_wifi_scan_stop();
            free(ap_list);
            free(blufi_ap_list);
        } else if ((uxBits & BLUFI_DAEMON_SEND_WIFI_REPORT_BIT) != 0) {
            wifi_mode_t mode;
            esp_wifi_get_mode(&mode);
            esp_blufi_extra_info_t info;
            memset(&info, 0, sizeof(esp_blufi_extra_info_t));
            memcpy(info.sta_bssid, gl_sta_bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = gl_sta_ssid;
            info.sta_ssid_len = gl_sta_ssid_len;
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
            break;
        };
    }

    blufi0_deinit();
    bt0_deinit();

    vTaskDelete(NULL);
}
