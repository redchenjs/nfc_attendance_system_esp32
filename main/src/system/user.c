/*
 * user.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "user/gui_daemon.h"
#include "user/key_daemon.h"
#include "user/ntp_daemon.h"
#include "user/led_daemon.h"
#include "user/nfc_daemon.h"
#include "user/audio_daemon.h"
#include "user/http2_daemon.h"

void user_init(void)
{
    xTaskCreate(nfc_daemon, "nfc_daemon", 5120, NULL, 5, NULL);
    xTaskCreate(ntp_daemon, "ntp_daemon", 2048, NULL, 5, NULL);

#if defined(CONFIG_ENABLE_SMART_CONFIG)
    xTaskCreate(key_daemon, "key_daemon", 2048, NULL, 5, NULL);
#endif

#if defined(CONFIG_ENABLE_LED)
    xTaskCreate(led_daemon, "led_daemon", 1024, NULL, 6, NULL);
#endif

#if defined(CONFIG_ENABLE_GUI)
    xTaskCreate(gui_daemon, "gui_daemon", 1024, NULL, 6, NULL);
#endif

#if defined(CONFIG_ENABLE_AUDIO)
    xTaskCreate(audio_daemon, "audio_daemon", 8448, NULL, 7, NULL);
#endif

    xTaskCreate(http2_daemon, "http2_daemon", 19200, NULL, 7, NULL);
}
