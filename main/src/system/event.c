/*
 * event.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_blufi_api.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"

#include "device/blufi.h"
#include "system/event.h"
#include "tasks/gui_task.h"
#include "tasks/sntp_client.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"

#define TAG "event"

bool gl_sta_connected = false;
uint8_t gl_sta_bssid[6];
uint8_t gl_sta_ssid[32];
int gl_sta_ssid_len;

EventGroupHandle_t system_event_group;
EventGroupHandle_t task_event_group;

esp_err_t system_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(system_event_group, WIFI_READY_BIT);
            gui_show_image(5);
            led_indicator_set_mode(2);
            xEventGroupWaitBits(task_event_group, SNTP_CLIENT_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
            gui_show_image(3);
            led_indicator_set_mode(1);

            if (blufi0_status == BLUFI_CONNECTED) {
                wifi_mode_t mode;
                esp_wifi_get_mode(&mode);
                esp_blufi_extra_info_t info;
                memset(&info, 0, sizeof(esp_blufi_extra_info_t));
                memcpy(info.sta_bssid, gl_sta_bssid, 6);
                info.sta_bssid_set = true;
                info.sta_ssid = gl_sta_ssid;
                info.sta_ssid_len = gl_sta_ssid_len;
                esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
            };
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            gl_sta_connected = true;
            memcpy(gl_sta_bssid, event->event_info.connected.bssid, 6);
            memcpy(gl_sta_ssid, event->event_info.connected.ssid, event->event_info.connected.ssid_len);
            gl_sta_ssid_len = event->event_info.connected.ssid_len;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            gl_sta_connected = false;
            memset(gl_sta_ssid, 0, 32);
            memset(gl_sta_bssid, 0, 6);
            gl_sta_ssid_len = 0;

            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(system_event_group, WIFI_READY_BIT);
            gui_show_image(0);
            led_indicator_set_mode(7);
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            if (blufi0_status == BLUFI_CONNECTED) {
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
                esp_blufi_ap_record_t * blufi_ap_list = (esp_blufi_ap_record_t *)malloc(ap_count * sizeof(esp_blufi_ap_record_t));
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
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void event_init(void)
{
    system_event_group = xEventGroupCreate();
    task_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(system_event_handler, NULL));
}
