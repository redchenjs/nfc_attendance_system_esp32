/*
 * app_main.c
 *
 *  Created on: 2018-03-11 15:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "core/os.h"
#include "core/app.h"

#include "chip/nvs.h"
#include "chip/spi.h"
#include "chip/i2s.h"
#include "chip/i2c.h"
#include "chip/wifi.h"
#include "chip/uart.h"

#include "board/pn532.h"

#include "user/ntp.h"
#include "user/led.h"
#include "user/gui.h"
#include "user/nfc_app.h"
#include "user/key_scan.h"
#include "user/http_app.h"
#include "user/audio_mp3.h"

static void core_init(void)
{
    app_print_info();

    os_init();
}

static void chip_init(void)
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

static void board_init(void)
{
    pn532_init();
}

static void user_init(void)
{
    ntp_init();

    nfc_app_init();

    http_app_init();

#ifdef CONFIG_ENABLE_SMARTCONFIG
    key_scan_init();
#endif

#ifdef CONFIG_ENABLE_LED
    led_init();
#endif

#ifdef CONFIG_ENABLE_GUI
    gui_init();
#endif

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    audio_mp3_init();
#endif
}

int app_main(void)
{
    core_init();

    chip_init();
    board_init();

    user_init();

    return 0;
}
