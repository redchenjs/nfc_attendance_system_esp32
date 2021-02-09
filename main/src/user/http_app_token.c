/*
 * http_app_token.c
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "core/os.h"
#include "chip/wifi.h"

#include "user/gui.h"
#include "user/http_app.h"
#include "user/audio_player.h"

#define TAG "http_app_token"

static char *token_string = NULL;

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
        cJSON *root = NULL;
        if (evt->data_len) {
            root = cJSON_Parse(evt->data);
            if (cJSON_HasObjectItem(root, "result")) {
                cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
                if (cJSON_IsTrue(result)) {
                    ESP_LOGW(TAG, "authentication done");
#ifdef CONFIG_ENABLE_GUI
                    gui_set_mode(GUI_MODE_IDX_GIF_DONE);
#endif
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                    audio_player_play_file(MP3_FILE_IDX_AUTH_DONE);
#endif
                } else {
                    ESP_LOGE(TAG, "authentication failed");
#ifdef CONFIG_ENABLE_GUI
                    gui_set_mode(GUI_MODE_IDX_GIF_FAIL);
#endif
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                    audio_player_play_file(MP3_FILE_IDX_AUTH_FAIL);
#endif
                }
            } else {
                ESP_LOGE(TAG, "invalid response");
                xEventGroupSetBits(user_event_group, HTTP_APP_TOKEN_FAIL_BIT);
            }
            cJSON_Delete(root);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        if (xEventGroupGetBits(user_event_group) & HTTP_APP_TOKEN_FAIL_BIT) {
#ifdef CONFIG_ENABLE_GUI
            gui_set_mode(GUI_MODE_IDX_GIF_FAIL);
#endif
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
            audio_player_play_file(MP3_FILE_IDX_ERROR_RSP);
#endif
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
    cJSON_AddNumberToObject(root, "request", HTTP_REQ_CODE_DEV_VERIFY_TOKEN);
    cJSON_AddStringToObject(root, "device_mac", wifi_mac_string);
    cJSON_AddStringToObject(root, "user_token", token_string);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

void http_app_verify_token(char *token)
{
    if (token == NULL) {
        return;
    }

    token_string = token;

    EventBits_t uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_TOKEN_RUN_BIT,
        HTTP_APP_TOKEN_DONE_BIT,
        30000 / portTICK_RATE_MS
    );

    if (!(uxBits & HTTP_APP_TOKEN_DONE_BIT)) {
        xEventGroupClearBits(user_event_group, HTTP_APP_TOKEN_RUN_BIT);
    }
}
