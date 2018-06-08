/*
 * ota_update.c
 *
 *  Created on: 2018-04-06 15:12
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "cJSON.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#include "device/wifi.h"
#include "system/event.h"
#include "system/firmware.h"
#include "user/gui_daemon.h"
#include "user/nfc_daemon.h"
#include "user/led_daemon.h"
#include "user/audio_daemon.h"

#define TAG "ota"

esp_err_t ota_update_event_handler(esp_http_client_event_t *evt)
{
    static const esp_partition_t *update_partition = NULL;
    static esp_ota_handle_t update_handle = 0;
    static long binary_file_length = 0;

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_OTA_FAILED_BIT);
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA: {
        if (evt->data_len) {
            EventBits_t uxBits = xEventGroupGetBits(daemon_event_group);
            if (!(uxBits & HTTP_DAEMON_OTA_RUN_BIT)) {
                xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_OTA_RUN_BIT);

                led_set_mode(3);
                gui_show_image(8);

                update_partition = esp_ota_get_next_update_partition(NULL);
                ESP_LOGI(TAG, "writing to partition subtype %d at offset 0x%x",
                            update_partition->subtype, update_partition->address);
                assert(update_partition != NULL);

                esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                    goto exit;
                }

                binary_file_length = 0;
            }
            esp_err_t err = esp_ota_write(update_handle, (const void *)evt->data, evt->data_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_write failed (%s)!", esp_err_to_name(err));
                xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_OTA_FAILED_BIT);
                goto exit;
            }
            binary_file_length += evt->data_len;
            ESP_LOGD(TAG, "have written image length %ld", binary_file_length);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        EventBits_t uxBits = xEventGroupGetBits(daemon_event_group);
        if (uxBits & HTTP_DAEMON_OTA_FAILED_BIT) {
            ESP_LOGE(TAG, "ota update failed");
            esp_ota_end(update_handle);
        } else if (binary_file_length != 0) {
            if (esp_ota_end(update_handle) != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_end failed!");
                goto exit;
            }
            esp_err_t err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
                goto exit;
            }
            ESP_LOGW(TAG, "prepare to restart system!");
            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);
            esp_restart();
        } else {
            ESP_LOGI(TAG, "no update found");
        }
exit:
        xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_OTA_READY_BIT);
        break;
    }
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void ota_update_prepare_data(char *buf, int len)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", 105);
    cJSON_AddStringToObject(root, "version", firmware_get_version());
    cJSON_AddStringToObject(root, "mac", wifi0_mac_str);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

void ota_update(void)
{
#if defined(CONFIG_ENABLE_OTA)
    xEventGroupClearBits(system_event_group, INPUT_READY_BIT);
    ESP_LOGI(TAG, "check firmware update, running %s", firmware_get_version());
    EventBits_t uxBits = xEventGroupSync(
        daemon_event_group,
        HTTP_DAEMON_OTA_READY_BIT,
        HTTP_DAEMON_OTA_FINISH_BIT,
        300000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP_DAEMON_OTA_FINISH_BIT) == 0) {
        xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_OTA_READY_BIT);
    }
    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
#endif
}
