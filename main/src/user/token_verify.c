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

#include "device/wifi.h"
#include "system/event.h"
#include "system/firmware.h"
#include "user/gui_daemon.h"
#include "user/nfc_daemon.h"
#include "user/led_daemon.h"
#include "user/audio_daemon.h"
#include "user/http2_daemon.h"
#include "user/http2_client.h"

#define TAG "token"

static char *data_ptr = NULL;

int token_verify_parse_data(struct http2c_handle *handle, const char *data, size_t len, int flags)
{
    cJSON *root   = NULL;
    cJSON *status = NULL;
    if (len) {
        root = cJSON_Parse(data);
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
            xEventGroupSetBits(daemon_event_group, HTTP2_DAEMON_TOKEN_FAILED_BIT);
        }
        cJSON_Delete(root);
    }
    if (flags == DATA_RECV_RST_STREAM) {
        xEventGroupClearBits(daemon_event_group, HTTP2_DAEMON_TOKEN_READY_BIT);
    }
    return 0;
}

int token_verify_prepare_data(struct http2c_handle *handle, char *buf, size_t length, uint32_t *data_flags)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", 100);
    cJSON_AddStringToObject(root, "token", data_ptr);
    cJSON_AddStringToObject(root, "mac", wifi0_mac_str);
    cJSON_PrintPreallocated(root, buf, length, 0);
    cJSON_Delete(root);
    (*data_flags) |= NGHTTP2_DATA_FLAG_EOF;
    return strlen(buf);
}

void token_verify(char *token)
{
    xEventGroupClearBits(system_event_group, INPUT_READY_BIT);
    data_ptr = token;
    EventBits_t uxBits = xEventGroupSync(
        daemon_event_group,
        HTTP2_DAEMON_TOKEN_READY_BIT,
        HTTP2_DAEMON_TOKEN_FINISH_BIT,
        30000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP2_DAEMON_TOKEN_FINISH_BIT) == 0) {
        xEventGroupClearBits(daemon_event_group, HTTP2_DAEMON_TOKEN_READY_BIT);
    }
    xEventGroupSetBits(system_event_group, INPUT_READY_BIT);
}
