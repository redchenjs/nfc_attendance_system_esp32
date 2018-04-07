/*
 * task.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tasks/gui_daemon.h"
#include "tasks/ntp_daemon.h"
#include "tasks/led_daemon.h"
#include "tasks/nfc_daemon.h"
#include "tasks/wifi_daemon.h"
#include "tasks/audio_daemon.h"
#include "tasks/blufi_daemon.h"
#include "tasks/http2_daemon.h"

void task_init(void)
{
#if defined(CONFIG_ENABLE_LED)
    xTaskCreate(led_daemon, "led_daemon", 1024, NULL, 6, NULL);
#endif

#if defined(CONFIG_ENABLE_GUI)
    xTaskCreate(gui_daemon, "gui_daemon", 1920, NULL, 6, NULL);
#endif

#if defined(CONFIG_ENABLE_AUDIO)
    xTaskCreate(audio_daemon, "audio_daemon", 8192, NULL, 6, NULL);
#endif

#if defined(CONFIG_ENABLE_BLUFI)
    xTaskCreate(blufi_daemon, "blufi_daemon", 1024, NULL, 6, NULL);
#endif

    xTaskCreate(nfc_daemon, "nfc_daemon", 5120, NULL, 4, NULL);
    xTaskCreate(ntp_daemon, "ntp_daemon", 1920, NULL, 5, NULL);
    xTaskCreate(wifi_daemon, "wifi_daemon", 1920, NULL, 5, NULL);
    xTaskCreate(http2_daemon, "http2_daemon", 19200, NULL, 5, NULL);
}
