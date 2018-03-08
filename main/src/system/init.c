/*
 * init.c
 *
 *  Created on: 2018-02-10 16:37
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "device/event.h"
#include "device/spi.h"
#include "device/i2s.h"
#include "device/gpio.h"
#include "device/uart.h"
#include "device/wifi.h"

#include "driver/led.h"
#include "buses/emdev.h"

void device_init(void)
{
    event0_init();
    spi1_init();
    i2s0_init();
    gpio0_init();
    uart1_init();
    wifi0_init();
}
void driver_init(void)
{
    led_init();
    emdev_init();
}

