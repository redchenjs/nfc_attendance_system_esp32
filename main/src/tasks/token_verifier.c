/*
 * token_verifier.c
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "cJSON.h"
#include "esp_log.h"

#include "device/wifi.h"
#include "system/event.h"
#include "tasks/gui_daemon.h"
#include "tasks/mp3_player.h"
#include "tasks/http2_client.h"
#include "tasks/nfc_initiator.h"
#include "tasks/led_indicator.h"
#include "tasks/token_verifier.h"

#define TAG "token_verifier"

static const uint8_t *cert_file_ptr[][2] =  {
                                                {cert0_pem_ptr, cert0_pem_end}  // "DigiCert Global Root CA"
                                            };
uint8_t cert_file_index = 0;

#define HTTP2_SERVER_URI        CONFIG_SERVER_URI
#define HTTP2_SERVER_POST_PATH  CONFIG_SERVER_POST_PATH

static char *data_ptr = NULL;

static int token_verifier_parse_data(struct http2c_handle *handle, const char *data, size_t len, int flags)
{
    cJSON *root   = NULL;
    cJSON *status = NULL;
    if (len) {
        root = cJSON_Parse(data);
        if (cJSON_HasObjectItem(root, "status")) {
            status = cJSON_GetObjectItemCaseSensitive(root, "status");
            if (cJSON_IsTrue(status)) {
                ESP_LOGW(TAG, "authentication success");
                gui_daemon_show_image(2);
                mp3_player_play_file(1);
            } else {
                ESP_LOGE(TAG, "authentication failed");
                gui_daemon_show_image(7);
                mp3_player_play_file(2);
            }
        } else {
            ESP_LOGE(TAG, "invalid response");
            gui_daemon_show_image(6);
            mp3_player_play_file(6);
        }
        cJSON_Delete(root);
    }
    if (flags == DATA_RECV_FRAME_COMPLETE || flags == DATA_RECV_RST_STREAM) {
        xEventGroupClearBits(task_event_group, TOKEN_VERIFIER_READY_BIT);
    }
    return 0;
}

static int token_verifier_prepare_data(struct http2c_handle *handle, char *buf, size_t length, uint32_t *data_flags)
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

void token_verifier_task(void *pvParameter)
{
    struct http2c_handle hd;

    while (1) {
        xEventGroupWaitBits(task_event_group, TOKEN_VERIFIER_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
        memset(&hd, 0, sizeof(hd));
        hd.ca_file_ptr = cert_file_ptr[cert_file_index][0];
        hd.ca_file_len = cert_file_ptr[cert_file_index][1] - cert_file_ptr[cert_file_index][0];
        if (http2_client_connect(&hd, HTTP2_SERVER_URI) != 0) {
            ESP_LOGE(TAG, "failed to connect");
            gui_daemon_show_image(6);
            mp3_player_play_file(3);
        } else {
            /* HTTP POST */
            http2_client_do_post(&hd, HTTP2_SERVER_POST_PATH, token_verifier_prepare_data, token_verifier_parse_data);
            while (1) {
                if (!(xEventGroupGetBits(task_event_group) & TOKEN_VERIFIER_READY_BIT)) {
                    break;
                }
                if (http2_client_execute(&hd) < 0) {
                    ESP_LOGE(TAG, "error in send/receive");
                    gui_daemon_show_image(6);
                    mp3_player_play_file(5);
                    break;
                }
                vTaskDelay(2 / portTICK_PERIOD_MS);
            }
        }
        http2_client_free(&hd);

        vTaskDelay(2000 / portTICK_RATE_MS);

        nfc_initiator_set_mode(1);
        led_indicator_set_mode(1);
        gui_daemon_show_image(3);

        xEventGroupClearBits(task_event_group, TOKEN_VERIFIER_READY_BIT);
    }
}

void token_verifier_verify_token(char *token)
{
    data_ptr = token;
    xEventGroupSetBits(task_event_group, TOKEN_VERIFIER_READY_BIT);
}
