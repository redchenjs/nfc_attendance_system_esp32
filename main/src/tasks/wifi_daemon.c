/*
 * wifi_daemon.c
 *
 *  Created on: 2018-04-01 15:04
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"

#include "system/event.h"
#include "tasks/gui_daemon.h"
#include "tasks/sntp_client.h"
#include "tasks/wifi_daemon.h"
#include "tasks/blufi_daemon.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"

#define TAG "wifi_daemon"

void wifi_daemon_task(void *pvParameter)
{
    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(
            task_event_group,
            WIFI_DAEMON_RECONNECT_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );
        if ((uxBits & NFC_INITIATOR_READY_BIT) != 0) {
            nfc_initiator_set_mode(0);
            led_indicator_set_mode(7);
            gui_daemon_show_image(0);
            int retry = 1;
            const int retry_count = 10;
            while (1) {
                ESP_LOGW(TAG, "waiting for wifi to be ready... (%d/%d)", retry, retry_count);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                uxBits = xEventGroupGetBits(system_event_group);
                if ((uxBits & WIFI_READY_BIT) != 0) {
                    break;
                }
                if (++retry > retry_count) {
                    ESP_LOGE(TAG, "can not wait to reboot...");
                    gui_daemon_show_image(4);
                    vTaskDelay(5000 / portTICK_RATE_MS);
                    esp_restart();
                }  
            }
        }
        xEventGroupClearBits(task_event_group, WIFI_DAEMON_RECONNECT_BIT);
    }
}

void wifi_daemon_reconnect(uint8_t mode)
{
    if (mode != 0) {
        xEventGroupSetBits(task_event_group, WIFI_DAEMON_RECONNECT_BIT);
    }
}
