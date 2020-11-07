/*
 * os.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "esp_smartconfig.h"

#include "freertos/event_groups.h"

#include "core/os.h"
#include "chip/wifi.h"

#include "user/gui.h"
#include "user/led.h"
#include "user/ntp.h"
#include "user/key.h"
#include "user/nfc_app.h"
#include "user/http_app_ota.h"

#define OS_SC_TAG  "os_sc"
#define OS_PWR_TAG "os_pwr"

EventGroupHandle_t wifi_event_group;
EventGroupHandle_t user_event_group;

static EventBits_t reset_wait_bits = OS_PWR_DUMMY_BIT;
static EventBits_t sleep_wait_bits = OS_PWR_DUMMY_BIT;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START: {
            EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
            if (!(uxBits & WIFI_CFG_BIT)) {
#ifdef CONFIG_ENABLE_GUI
                gui_set_mode(GUI_MODE_IDX_GIF_WIFI);
#endif
                esp_wifi_connect();
            }
            break;
        }
        case WIFI_EVENT_STA_CONNECTED:
            break;
        case WIFI_EVENT_STA_DISCONNECTED: {
            EventBits_t uxBits = xEventGroupGetBits(wifi_event_group);
            if (uxBits & WIFI_RDY_BIT) {
#ifdef CONFIG_ENABLE_GUI
                gui_set_mode(GUI_MODE_IDX_GIF_PWR);
                vTaskDelay(2000 / portTICK_RATE_MS);
#endif
                os_pwr_reset_wait(OS_PWR_DUMMY_BIT);
            }
            if (!(uxBits & WIFI_CFG_BIT)) {
                esp_wifi_connect();
            }
            xEventGroupClearBits(wifi_event_group, WIFI_RDY_BIT);
            break;
        }
        default:
            break;
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP: {
            xEventGroupSetBits(wifi_event_group, WIFI_RDY_BIT);

            key_set_scan_mode(KEY_SCAN_MODE_IDX_OFF);

            ntp_sync_time();
#ifdef CONFIG_ENABLE_OTA
            http_app_check_for_updates();
#endif
#ifdef CONFIG_ENABLE_GUI
            gui_set_mode(GUI_MODE_IDX_GIF_SCAN);
#endif
#ifdef CONFIG_ENABLE_LED
            led_set_mode(LED_MODE_IDX_BLINK_S0);
#endif
            nfc_app_set_mode(NFC_APP_MODE_IDX_ON);

            break;
        }
        default:
            break;
    }
}

static void sc_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    static wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    switch (event_id) {
        case SC_EVENT_SCAN_DONE:
            ESP_LOGI(OS_SC_TAG, "scan done");
            break;
        case SC_EVENT_FOUND_CHANNEL:
            ESP_LOGI(OS_SC_TAG, "found channel");
            break;
        case SC_EVENT_GOT_SSID_PSWD:
            ESP_LOGI(OS_SC_TAG, "got ssid and passwd");

#ifdef CONFIG_ENABLE_LED
            led_set_mode(LED_MODE_IDX_PULSE_D1);
#endif

            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
            wifi_config.sta.bssid_set = evt->bssid_set;
            if (wifi_config.sta.bssid_set == true) {
                memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
            }
            ESP_LOGI(OS_SC_TAG, "ssid: %s", wifi_config.sta.ssid);
            ESP_LOGI(OS_SC_TAG, "password: %s", wifi_config.sta.password);

            esp_wifi_disconnect();
            esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
            esp_wifi_connect();
            break;
        case SC_EVENT_SEND_ACK_DONE:
            ESP_LOGI(OS_SC_TAG, "ack done");
            esp_smartconfig_stop();
            xEventGroupClearBits(wifi_event_group, WIFI_CFG_BIT);
            break;
        default:
            break;
    }
}

static void os_pwr_task_handle(void *pvParameters)
{
    ESP_LOGI(OS_PWR_TAG, "started.");

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            OS_PWR_RESET_BIT | OS_PWR_SLEEP_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & OS_PWR_RESET_BIT) {
            if (reset_wait_bits) {
                ESP_LOGW(OS_PWR_TAG, "waiting for unfinished jobs....");

                vTaskDelay(500 / portTICK_RATE_MS);

                xEventGroupWaitBits(
                    user_event_group,
                    reset_wait_bits,
                    pdFALSE,
                    pdTRUE,
                    portMAX_DELAY
                );
            }

            ESP_LOGW(OS_PWR_TAG, "reset now");
            esp_restart();
        } else if (uxBits & OS_PWR_SLEEP_BIT) {
            if (sleep_wait_bits) {
                ESP_LOGW(OS_PWR_TAG, "waiting for unfinished jobs....");

                vTaskDelay(500 / portTICK_RATE_MS);

                xEventGroupWaitBits(
                    user_event_group,
                    sleep_wait_bits,
                    pdFALSE,
                    pdTRUE,
                    portMAX_DELAY
                );
            }

            ESP_LOGW(OS_PWR_TAG, "sleep now");
            esp_deep_sleep_start();
        }
    }
}

void os_pwr_reset_wait(EventBits_t bits)
{
    reset_wait_bits = bits;
    xEventGroupSetBits(user_event_group, OS_PWR_RESET_BIT);
}

void os_pwr_sleep_wait(EventBits_t bits)
{
    sleep_wait_bits = bits;
    xEventGroupSetBits(user_event_group, OS_PWR_SLEEP_BIT);
}

void os_init(void)
{
    wifi_event_group = xEventGroupCreate();
    user_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL));

    xTaskCreatePinnedToCore(os_pwr_task_handle, "osPwrT", 2048, NULL, 5, NULL, 0);
}
