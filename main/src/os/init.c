/*
 * init.c
 *
 *  Created on: 2018-02-10 16:37
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "os/event.h"

#include "chip/nvs.h"
#include "chip/spi.h"
#include "chip/i2s.h"
#include "chip/i2c.h"
#include "chip/wifi.h"
#include "chip/uart.h"

#include "board/pn532.h"

#include "user/nfc_app.h"
#include "user/gui.h"
#include "user/key.h"
#include "user/ntp.h"
#include "user/led.h"
#include "user/http.h"
#include "user/audio.h"

void os_init(void)
{
    event_init();
}

void chip_init(void)
{
    nvs_init();

    wifi_init();

#ifdef CONFIG_PN532_IFCE_UART
    uart1_init();
#else
    i2c0_init();
#endif

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    i2s0_init();
#endif

#ifdef CONFIG_ENABLE_GUI
    hspi_init();
#endif
}

void board_init(void)
{
    pn532_init();
}

void user_init(void)
{
    nfc_app_init();

    ntp_init();

    http_init();

#ifdef CONFIG_ENABLE_SMARTCONFIG
    key_init();
#endif

#ifdef CONFIG_ENABLE_LED
    led_init();
#endif

#ifdef CONFIG_ENABLE_GUI
    gui_init();
#endif

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    audio_init();
#endif
}
