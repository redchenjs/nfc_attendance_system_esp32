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
#include "tasks/ota_update.h"
#include "tasks/gui_daemon.h"
#include "tasks/led_daemon.h"
#include "tasks/nfc_daemon.h"
#include "tasks/ntp_daemon.h"
#include "tasks/wifi_daemon.h"
#include "tasks/blufi_daemon.h"

#define TAG "wifi"

void wifi_daemon(void *pvParameter)
{
    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(
            daemon_event_group,
            WIFI_DAEMON_DISCONNECTED_BIT | WIFI_DAEMON_GOT_IP_BIT | WIFI_DAEMON_START_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if ((uxBits & WIFI_DAEMON_DISCONNECTED_BIT) != 0) {
            xEventGroupClearBits(system_event_group, WIFI_READY_BIT);
            nfc_set_mode(0);
            gui_show_image(4);
            vTaskDelay(2000 / portTICK_RATE_MS);
            esp_restart();
        } else if ((uxBits & WIFI_DAEMON_GOT_IP_BIT) != 0) {
            xEventGroupSetBits(system_event_group, WIFI_READY_BIT);
            blufi_response();
            ntp_sync_time();
            ota_update();
            nfc_set_mode(1);
        } else {
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
    }
}

void wifi_on_disconnected(void)
{
    EventBits_t uxBits;
#if defined(CONFIG_ENABLE_BLUFI)
    uxBits = xEventGroupGetBits(system_event_group);
    if ((uxBits & BLUFI_READY_BIT) == 0) {
#endif
        uxBits = xEventGroupGetBits(daemon_event_group);
        if ((uxBits & NFC_DAEMON_READY_BIT) != 0) {
            xEventGroupSetBits(daemon_event_group, WIFI_DAEMON_DISCONNECTED_BIT);
        } else {
            xEventGroupSetBits(daemon_event_group, WIFI_DAEMON_START_BIT);
        }
#if defined(CONFIG_ENABLE_BLUFI)
    }
#endif
}

void wifi_on_got_ip(void)
{
    xEventGroupSetBits(daemon_event_group, WIFI_DAEMON_GOT_IP_BIT);
}

void wifi_on_start(void)
{
    xEventGroupSetBits(daemon_event_group, WIFI_DAEMON_START_BIT);
}
