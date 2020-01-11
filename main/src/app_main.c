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
#include "chip/wifi.h"
#include "chip/uart.h"

#include "board/pn532.h"

#include "user/ntp.h"
#include "user/led.h"
#include "user/gui.h"
#include "user/key.h"
#include "user/nfc_app.h"
#include "user/http_app.h"
#include "user/audio_player.h"

static void core_init(void)
{
    app_print_info();

    os_init();
}

static void chip_init(void)
{
    nvs_init();

    wifi_init();

    uart1_init();

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    i2s_output_init();
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

#ifdef CONFIG_ENABLE_SC_KEY
    key_init();
#endif

#ifdef CONFIG_ENABLE_LED
    led_init();
#endif

#ifdef CONFIG_ENABLE_GUI
    gui_init();
#endif

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
    audio_player_init();
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
