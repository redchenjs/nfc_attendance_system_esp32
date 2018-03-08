/*
 * main_task.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "tasks/main_task.h"
#include "tasks/mp3_player.h"
#include "tasks/sntp_client.h"
#include "tasks/oled_display.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"
#include "tasks/token_verifier.h"

EventGroupHandle_t task_event_group;
EventGroupHandle_t system_event_group;

void main_task(void)
{
    task_event_group = xEventGroupCreate();
    system_event_group = xEventGroupCreate();
    xTaskCreate(&mp3_player_task, "mp3_player_task", 8192, NULL, 5, NULL);
    xTaskCreate(&sntp_client_task, "sntp_client_task", 2048, NULL, 5, NULL);
    xTaskCreate(&oled_display_task, "oled_display_task", 2048, NULL, 5, NULL);
    xTaskCreate(&led_indicator_task, "led_indicator_task", 1024, NULL, 5, NULL);
    xTaskCreate(&nfc_initiator_task, "nfc_initiator_task", 5120, NULL, 5, NULL);
    xTaskCreate(&token_verifier_task, "token_verifier_task", 19200, NULL, 5, NULL);
}
