#include "buses/emdev.h"
#include "inc/device/led.h"
#include "inc/driver/ssd1306.h"
/*
 * init.c
 *
 *  Created on: 2017-4-16
 *      Author: redchenjs
 */
nfc_emdev emdev;

void device_init(void)
{
    emdev_init(&emdev);
    
    led_init();
}
void driver_init(void)
{
    ssd1306_init();
}

