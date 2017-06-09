#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inc/device/led.h"
/*
 * app_led.c
 *
 *  Created on: 2017-6-4
 *      Author: redchenjs
 */

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Main Program
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void task_led(void *pvParameter)
{
    static unsigned char i;
    
    while (1) {
        if (++i % 2) {
            led_on();
        }
        else {
            led_off();
        }
        vTaskDelay(200 / portTICK_RATE_MS);
    }
}
