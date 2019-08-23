/*
 * http_app_ota.c
 *
 *  Created on: 2018-04-06 15:12
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "core/os.h"
#include "core/app.h"
#include "chip/wifi.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio_mp3.h"

#define TAG "http_app_ota"

static uint8_t first_time = 1;

esp_err_t http_app_ota_event_handler(esp_http_client_event_t *evt)
{
    static const esp_partition_t *update_partition = NULL;
    static esp_ota_handle_t update_handle = 0;
    static long binary_file_length = 0;

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA: {
        if (evt->data_len) {
            if (first_time) {
                first_time = 0;

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
                ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
                goto exit;
            }
            binary_file_length += evt->data_len;
            ESP_LOGD(TAG, "have written image length %ld", binary_file_length);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        if (binary_file_length != 0) {
            if (esp_ota_end(update_handle) != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_end failed");
                goto exit;
            }
            esp_err_t err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
                goto exit;
            }
            ESP_LOGW(TAG, "prepare to restart system");
            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);
            esp_restart();
        } else {
            ESP_LOGI(TAG, "no update found");
        }
        break;
    }
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
exit:
        xEventGroupSetBits(user_event_group, HTTP_APP_OTA_FAILED_BIT);
        break;
    }
    return ESP_OK;
}

void http_app_ota_prepare_data(char *buf, int len)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", 105);
    cJSON_AddStringToObject(root, "version", app_get_version());
    cJSON_AddStringToObject(root, "mac", wifi_mac_string);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

void http_app_ota_update(void)
{
#ifdef CONFIG_ENABLE_OTA
    xEventGroupClearBits(os_event_group, INPUT_READY_BIT);
    ESP_LOGI(TAG, "checking for firmware update...");
    EventBits_t uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_OTA_RUN_BIT,
        HTTP_APP_OTA_READY_BIT,
        60000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP_APP_OTA_READY_BIT) == 0) {
        xEventGroupClearBits(user_event_group, HTTP_APP_OTA_RUN_BIT);
    }
    first_time = 1;
    xEventGroupSetBits(os_event_group, INPUT_READY_BIT);
#endif
}
