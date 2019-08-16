/*
 * http_app_token.c
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "os/core.h"
#include "os/firmware.h"
#include "chip/wifi.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio_mp3.h"

#define TAG "http_app_token"

static char *data_ptr = NULL;

esp_err_t http_app_token_event_handler(esp_http_client_event_t *evt)
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
                    audio_mp3_play(1);
                } else {
                    ESP_LOGE(TAG, "authentication failed");
                    gui_show_image(6);
                    audio_mp3_play(2);
                }
            } else {
                ESP_LOGE(TAG, "invalid response");
                xEventGroupSetBits(user_event_group, HTTP_APP_TOKEN_FAILED_BIT);
            }
            cJSON_Delete(root);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & HTTP_APP_TOKEN_FAILED_BIT) {
            gui_show_image(6);
            audio_mp3_play(6);
        }
        break;
    }
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void http_app_token_prepare_data(char *buf, int len)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", 100);
    cJSON_AddStringToObject(root, "token", data_ptr);
    cJSON_AddStringToObject(root, "mac", wifi_mac_string);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

void http_app_token_verify(char *token)
{
    xEventGroupClearBits(os_event_group, INPUT_READY_BIT);
    data_ptr = token;
    EventBits_t uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_TOKEN_RUN_BIT,
        HTTP_APP_TOKEN_READY_BIT,
        30000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP_APP_TOKEN_READY_BIT) == 0) {
        xEventGroupClearBits(user_event_group, HTTP_APP_TOKEN_RUN_BIT);
    }
    xEventGroupSetBits(os_event_group, INPUT_READY_BIT);
}
