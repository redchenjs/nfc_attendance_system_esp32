/*
 * event.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"

#include "system/event.h"
#include "user/wifi_daemon.h"

#define TAG "event"

EventGroupHandle_t system_event_group;
EventGroupHandle_t daemon_event_group;

static esp_err_t system_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            wifi_on_start();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            wifi_on_got_ip();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            wifi_on_disconnected();
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            break;
        default:
            break;
    }
    return ESP_OK;
}

void event_init(void)
{
    system_event_group = xEventGroupCreate();
    daemon_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(system_event_handler, NULL));
}
