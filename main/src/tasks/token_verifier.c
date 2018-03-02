/*
 * token_verifier.c
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "device/wifi.h"

#include "cJSON.h"

#include "tasks/mp3_player.h"
#include "tasks/http2_client.h"
#include "tasks/oled_display.h"
#include "tasks/nfc_initiator.h"
#include "tasks/led_indicator.h"
#include "tasks/token_verifier.h"

#define TAG "token_verifier"

#define HTTP2_CLIENT_CA_CERT "/spiffs/cert/cert0.pem"
#define HTTP2_SERVER_URI     "https://redchenjs.vicp.net"
#define HTTP2_POST_PATH      "/iot/"

uint8_t token_verifier_status = TOKEN_VERIFIER_STOPPED;

static char *data_ptr = NULL;

void token_verifier_verify_token(char *token)
{
    data_ptr = token;

    if (token_verifier_status == TOKEN_VERIFIER_RUNNING) {
        ESP_LOGW(TAG, "token verifier is running, waiting");
    }
    while (token_verifier_status != TOKEN_VERIFIER_STOPPED);
    xTaskCreate(&token_verifier_task, "token_verifier_task", (1024 * 32), NULL, 5, NULL);
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
                vTaskDelay(3000 / portTICK_RATE_MS);
            } else {
                ESP_LOGE(TAG, "authentication failed");
                mp3_player_play_file(2);
            }
        } else {
            ESP_LOGE(TAG, "invalid response");
            mp3_player_play_file(6);
        }
        oled_display_show_image(3);
        cJSON_Delete(root);
    }
    if (flags == DATA_RECV_FRAME_COMPLETE || flags == DATA_RECV_RST_STREAM) {
        token_verifier_status = TOKEN_VERIFIER_STOPPING;
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

    xEventGroupWaitBits(wifi0_event_group, WIFI0_CONNECTED_BIT,
                        false, true, portMAX_DELAY);

    token_verifier_status = TOKEN_VERIFIER_RUNNING;

    if (http2_client_connect(&hd, HTTP2_SERVER_URI, HTTP2_CLIENT_CA_CERT) != 0) {
        ESP_LOGE(TAG, "failed to connect");
        mp3_player_play_file(3);
        token_verifier_status = TOKEN_VERIFIER_STOPPING;
    } else {
        /* HTTP POST  */
        http2_client_do_post(&hd, HTTP2_POST_PATH, token_verifier_prepare_data, token_verifier_parse_data);

        uint16_t execute_cnt = 0;
        while (token_verifier_status == TOKEN_VERIFIER_RUNNING) {
            if (http2_client_execute(&hd) < 0) {
                token_verifier_status = TOKEN_VERIFIER_STOPPING;
                ESP_LOGE(TAG, "error in send/receive");
                oled_display_show_image(3);
                mp3_player_play_file(5);
                break;
            }

            if (execute_cnt++ > 2500) {
                token_verifier_status = TOKEN_VERIFIER_STOPPING;
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

    token_verifier_status = TOKEN_VERIFIER_STOPPED;

    vTaskDelete(NULL);
}
