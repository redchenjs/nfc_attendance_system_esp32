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

#include "device/wifi.h"

#include "tasks/sntp_client.h"
#include "tasks/oled_display.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"

#define TAG "event-0"

esp_err_t event0_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi0_event_group, WIFI0_CONNECTED_BIT);
            if (sntp_client_status == SNTP_TIME_SET) {
                oled_display_show_image(3);
                led_indicator_set_mode(1);
                nfc_initiator_set_mode(1);
            } else {
                oled_display_show_image(5);
                led_indicator_set_mode(2);
            }
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_ERROR_CHECK(esp_wifi_connect());
            xEventGroupClearBits(wifi0_event_group, WIFI0_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void event0_init(void)
{
    ESP_ERROR_CHECK(esp_event_loop_init(event0_handler, NULL));
}
