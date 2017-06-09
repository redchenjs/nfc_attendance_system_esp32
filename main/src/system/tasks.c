#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "inc/tasks/task_nfc.h"
/*
 * tasks.c
 *
 *  Created on: 2017-6-6
 *      Author: redchenjs
 */

void tasks_init(void)
{
    xTaskCreate(&task_nfc, "task_nfc", 5120, NULL, 4, NULL);
}
