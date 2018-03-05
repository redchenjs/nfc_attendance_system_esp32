/*
 * main_task.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tasks/sntp_client.h"
#include "tasks/oled_display.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"

void main_task(void)
{
    xTaskCreate(&sntp_client_task, "sntp_client_task", 2048, NULL, 5, NULL);
    xTaskCreate(&oled_display_task, "oled_display_task", 1536, NULL, 5, NULL);
    xTaskCreate(&led_indicator_task, "led_indicator_task", 1024, NULL, 5, &led_indicator_task_handle);
    xTaskCreate(&nfc_initiator_task, "nfc_initiator_task", 5120, NULL, 5, &nfc_initiator_task_handle);
}
