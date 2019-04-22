/*
 * user.c
 *
 *  Created on: 2018-02-16 18:00
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "user/gui.h"
#include "user/key.h"
#include "user/ntp.h"
#include "user/led.h"
#include "user/nfc.h"
#include "user/http.h"
#include "user/audio.h"

void user_init(void)
{
    xTaskCreate(nfc_daemon, "nfc_daemon", 5120, NULL, 5, NULL);
    xTaskCreate(ntp_daemon, "ntp_daemon", 2048, NULL, 5, NULL);

#ifdef CONFIG_ENABLE_SMARTCONFIG
    xTaskCreate(key_daemon, "key_daemon", 2048, NULL, 5, NULL);
#endif

#ifdef CONFIG_ENABLE_LED
    xTaskCreate(led_daemon, "led_daemon", 1024, NULL, 6, NULL);
#endif

#ifdef CONFIG_ENABLE_GUI
    xTaskCreate(gui_daemon, "gui_daemon", 1024, NULL, 6, NULL);
#endif

#ifdef CONFIG_ENABLE_AUDIO
    xTaskCreate(audio_daemon, "audio_daemon", 8448, NULL, 7, NULL);
#endif

    xTaskCreate(http_daemon, "http_daemon", 5120, NULL, 7, NULL);
}
