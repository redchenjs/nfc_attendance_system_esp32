/*
 * token_verify.c
 *
 *  Created on: 2018-04-06 15:09
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

#define TAG "token"

static char *data_ptr = NULL;

esp_err_t token_verify_event_handler(esp_http_client_event_t *evt)
{
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
        cJSON *root   = NULL;
        cJSON *status = NULL;
        if (evt->data_len) {
            root = cJSON_Parse(evt->data);
            if (cJSON_HasObjectItem(root, "status")) {
                status = cJSON_GetObjectItemCaseSensitive(root, "status");
                if (cJSON_IsTrue(status)) {
                    ESP_LOGW(TAG, "authentication success");
                    gui_show_image(2);
                    audio_play_file(1);
                } else {
                    ESP_LOGE(TAG, "authentication failed");
                    gui_show_image(6);
                    audio_play_file(2);
                }
            } else {
                ESP_LOGE(TAG, "invalid response");
                gui_show_image(6);
                audio_play_file(6);
                xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_TOKEN_FAILED_BIT);
            }
            cJSON_Delete(root);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH:
        xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_TOKEN_READY_BIT);
        break;
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void token_verify_prepare_data(char *buf, int len)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", 100);
    cJSON_AddStringToObject(root, "token", data_ptr);
    cJSON_AddStringToObject(root, "mac", wifi_mac_str);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

void token_verify(char *token)
{
    xEventGroupClearBits(system_event_group, INPUT_READY_BIT);
    data_ptr = token;
    EventBits_t uxBits = xEventGroupSync(
        daemon_event_group,
        HTTP_DAEMON_TOKEN_READY_BIT,
        HTTP_DAEMON_TOKEN_FINISH_BIT,
        30000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP_DAEMON_TOKEN_FINISH_BIT) == 0) {
        xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_TOKEN_READY_BIT);
    }
    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
}
