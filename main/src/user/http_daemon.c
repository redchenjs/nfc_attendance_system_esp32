/*
 * http_daemon.c
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"

#include "device/wifi.h"
#include "system/event.h"
#include "system/firmware.h"
#include "user/gui_daemon.h"
#include "user/nfc_daemon.h"
#include "user/led_daemon.h"
#include "user/ota_update.h"
#include "user/token_verify.h"
#include "user/audio_daemon.h"
#include "user/http_daemon.h"

#define TAG "http"

void http_daemon(void *pvParameter)
{
    char post_data[128] = {0};
    char server_url[80] = {0};
    esp_http_client_config_t config;

    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(
            daemon_event_group,
            HTTP_DAEMON_TOKEN_READY_BIT | HTTP_DAEMON_OTA_READY_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        led_set_mode(4);
        gui_show_image(1);

        memset(&config, 0, sizeof(config));
        memset(post_data, 0, sizeof(post_data));
        memset(server_url, 0, sizeof(server_url));

#if defined(CONFIG_TRANSFER_PROTOCOL_HTTP)
        strcpy(server_url, "http://");
#else
        strcpy(server_url, "https://");
#endif
        strcat(server_url, CONFIG_SERVER_URL);
        config.url = (const char *)server_url;
#if defined(CONFIG_ENABLE_SERVER_CERT_VERIFY)
        config.cert_pem = cert0_pem_ptr;
#endif

        if (uxBits & HTTP_DAEMON_TOKEN_READY_BIT) {
            config.event_handler = token_verify_event_handler;
            token_verify_prepare_data(post_data, sizeof(post_data));
            xEventGroupClearBits(
                daemon_event_group,
                HTTP_DAEMON_TOKEN_FAILED_BIT | HTTP_DAEMON_TOKEN_FINISH_BIT
            );
        } else {
            config.event_handler = ota_update_event_handler;
            ota_update_prepare_data(post_data, sizeof(post_data));
            xEventGroupClearBits(
                daemon_event_group,
                HTTP_DAEMON_OTA_FAILED_BIT | HTTP_DAEMON_OTA_FINISH_BIT | HTTP_DAEMON_OTA_RUN_BIT
            );
        }
        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);
        if (err != ESP_OK) {
#if defined(CONFIG_TRANSFER_PROTOCOL_HTTP)
            ESP_LOGE(TAG, "error perform http request %s", esp_err_to_name(err));
#else
            ESP_LOGE(TAG, "error perform https request %s", esp_err_to_name(err));
#endif
            gui_show_image(6);
            audio_play_file(5);
        }
        esp_http_client_cleanup(client);

        vTaskDelay(2000 / portTICK_RATE_MS);

        led_set_mode(1);
        gui_show_image(3);

        if (uxBits & HTTP_DAEMON_TOKEN_READY_BIT) {
            xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_TOKEN_FINISH_BIT);
            xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_TOKEN_READY_BIT);
        } else {
            xEventGroupSetBits(daemon_event_group, HTTP_DAEMON_OTA_FINISH_BIT);
            xEventGroupClearBits(daemon_event_group, HTTP_DAEMON_OTA_READY_BIT);
        }
    }
}
