/*
 * task.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tasks/gui_task.h"
#include "tasks/mp3_player.h"
#include "tasks/sntp_client.h"
#include "tasks/led_indicator.h"
#include "tasks/nfc_initiator.h"
#include "tasks/token_verifier.h"

void task_init(void)
{
#if defined(CONFIG_ENABLE_GUI)
    xTaskCreate(gui_task, "gui_task", 2048, NULL, 5, NULL);
#endif
    xTaskCreate(mp3_player_task, "mp3_player_task", 8192, NULL, 5, NULL);
    xTaskCreate(sntp_client_task, "sntp_client_task", 2048, NULL, 5, NULL);
    xTaskCreate(led_indicator_task, "led_indicator_task", 1024, NULL, 5, NULL);
    xTaskCreate(nfc_initiator_task, "nfc_initiator_task", 5120, NULL, 5, NULL);
    xTaskCreate(token_verifier_task, "token_verifier_task", 19200, NULL, 5, NULL);
}
