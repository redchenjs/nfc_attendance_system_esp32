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
#include "tasks/main_task.h"
#include "tasks/mp3_player.h"
#include "tasks/http2_client.h"
#include "tasks/oled_display.h"
#include "tasks/nfc_initiator.h"
#include "tasks/led_indicator.h"
#include "tasks/token_verifier.h"

#define TAG "token_verifier"

static const uint8_t *cert_file_ptr[][2] =  {
                                                {cert0_pem_ptr, cert0_pem_end}  // "DigiCert Global Root CA"
                                            };
uint8_t cert_file_index = 0;

#define HTTP2_SERVER_URI     "https://redchenjs.vicp.net"
#define HTTP2_POST_PATH      "/iot/"

static char *data_ptr = NULL;

void token_verifier_verify_token(char *token)
{
    data_ptr = token;
    xEventGroupSetBits(task_event_group, TOKEN_VERIFIER_READY_BIT);
}

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
                oled_display_show_image(2);
                mp3_player_play_file(1);
            } else {
                ESP_LOGE(TAG, "authentication failed");
                oled_display_show_image(7);
                mp3_player_play_file(2);
            }
        } else {
            ESP_LOGE(TAG, "invalid response");
            oled_display_show_image(6);
            mp3_player_play_file(6);
        }
        cJSON_Delete(root);
        vTaskDelay(2000 / portTICK_RATE_MS);
        oled_display_show_image(3);
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

        hd.ca_file_ptr = cert_file_ptr[cert_file_index][0];
        hd.ca_file_len = cert_file_ptr[cert_file_index][1] - cert_file_ptr[cert_file_index][0];
        if (http2_client_connect(&hd, HTTP2_SERVER_URI) != 0) {
            ESP_LOGE(TAG, "failed to connect");
            oled_display_show_image(3);
            mp3_player_play_file(3);
        } else {
            /* HTTP POST  */
            http2_client_do_post(&hd, HTTP2_POST_PATH, token_verifier_prepare_data, token_verifier_parse_data);

            uint16_t execute_cnt = 0;
            while (1) {
                if (!(xEventGroupGetBits(task_event_group) & TOKEN_VERIFIER_READY_BIT)) {
                    break;
                }
                if (http2_client_execute(&hd) < 0) {
                    ESP_LOGE(TAG, "error in send/receive");
                    oled_display_show_image(3);
                    mp3_player_play_file(5);
                    break;
                }
                if (execute_cnt++ > 2500) {
                    ESP_LOGE(TAG, "execute timeout");
                    oled_display_show_image(3);
                    mp3_player_play_file(4);
                    break;
                } else {
                    vTaskDelay(2 / portTICK_PERIOD_MS);
                }
            }
        }
        http2_client_free(&hd);

        nfc_initiator_set_mode(1);
        led_indicator_set_mode(1);

        xEventGroupClearBits(task_event_group, TOKEN_VERIFIER_READY_BIT);
    }
}
