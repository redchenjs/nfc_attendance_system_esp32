/*
 * http2_daemon.c
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"

#include "device/wifi.h"
#include "system/event.h"
#include "system/firmware.h"
#include "user/gui_daemon.h"
#include "user/nfc_daemon.h"
#include "user/led_daemon.h"
#include "user/ota_update.h"
#include "user/token_verify.h"
#include "user/audio_daemon.h"
#include "user/http2_daemon.h"
#include "user/http2_client.h"

#define TAG "http2"

void http2_daemon(void *pvParameter)
{
    while (1) {
        EventBits_t uxBitsPrev = xEventGroupWaitBits(
            daemon_event_group,
            HTTP2_DAEMON_TOKEN_READY_BIT | HTTP2_DAEMON_OTA_READY_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        led_set_mode(4);
        gui_show_image(1);

        struct http2c_handle hd;
        if (http2_client_connect(&hd, CONFIG_SERVER_URI) != 0) {
            ESP_LOGE(TAG, "failed to connect");
            if (uxBitsPrev & HTTP2_DAEMON_TOKEN_READY_BIT) {
                gui_show_image(6);
                audio_play_file(3);
            }
        } else {
            /* HTTP POST */
            if (uxBitsPrev & HTTP2_DAEMON_TOKEN_READY_BIT) {
                http2_client_do_post(
                    &hd,
                    CONFIG_SERVER_POST_PATH,
                    token_verify_prepare_data,
                    token_verify_parse_data
                );
                xEventGroupClearBits(
                    daemon_event_group,
                    HTTP2_DAEMON_TOKEN_FAILED_BIT | HTTP2_DAEMON_TOKEN_FINISH_BIT
                );
            } else {
                http2_client_do_post(
                    &hd,
                    CONFIG_SERVER_POST_PATH,
                    ota_update_prepare_data,
                    ota_update_parse_data
                );
                xEventGroupClearBits(
                    daemon_event_group,
                    HTTP2_DAEMON_OTA_FAILED_BIT | HTTP2_DAEMON_OTA_FINISH_BIT | HTTP2_DAEMON_OTA_RUN_BIT
                );
            }
            EventBits_t uxBits;
            while (1) {
                uxBits = xEventGroupGetBits(system_event_group);
                if (!(uxBits & WIFI_READY_BIT) || (http2_client_execute(&hd) < 0)) {
                    ESP_LOGE(TAG, "error in send/receive");
                    if (uxBitsPrev & HTTP2_DAEMON_TOKEN_READY_BIT) {
                        gui_show_image(6);
                        audio_play_file(5);
                    }
                    break;
                }
                vTaskDelay(2 / portTICK_PERIOD_MS);
                uxBits = xEventGroupGetBits(daemon_event_group);
                if (!(uxBits & HTTP2_DAEMON_TOKEN_READY_BIT) && !(uxBits & HTTP2_DAEMON_OTA_READY_BIT)) {
                    break;
                }
            }
        }
        http2_client_free(&hd);
        vTaskDelay(2000 / portTICK_RATE_MS);

        led_set_mode(1);
        gui_show_image(3);

        if (uxBitsPrev & HTTP2_DAEMON_TOKEN_READY_BIT) {
            xEventGroupSetBits(daemon_event_group, HTTP2_DAEMON_TOKEN_FINISH_BIT);
            xEventGroupClearBits(daemon_event_group, HTTP2_DAEMON_TOKEN_READY_BIT);
        } else {
            xEventGroupSetBits(daemon_event_group, HTTP2_DAEMON_OTA_FINISH_BIT);
            xEventGroupClearBits(daemon_event_group, HTTP2_DAEMON_OTA_READY_BIT);
        }
    }
}
