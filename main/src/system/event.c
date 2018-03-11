/*
 * event.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"

#include "system/task.h"
#include "system/event.h"
#include "tasks/gui_task.h"
#include "tasks/sntp_client.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"

#define TAG "event"

EventGroupHandle_t system_event_group;
EventGroupHandle_t task_event_group;

esp_err_t system_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(system_event_group, WIFI_READY_BIT);
            gui_show_image(5);
            led_indicator_set_mode(2);
            xEventGroupWaitBits(task_event_group, SNTP_CLIENT_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
            gui_show_image(3);
            led_indicator_set_mode(1);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(system_event_group, WIFI_READY_BIT);
            gui_show_image(0);
            led_indicator_set_mode(7);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void event_init(void)
{
    system_event_group = xEventGroupCreate();
    task_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(system_event_handler, NULL));
}