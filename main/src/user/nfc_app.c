/*
 * nfc_app.c
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"

#include "nfc/nfc.h"

#include "core/os.h"
#include "board/pn532.h"

#include "user/gui.h"
#include "user/ntp.h"
#include "user/led.h"
#include "user/nfc_app.h"
#include "user/audio_player.h"
#include "user/http_app_token.h"

#define TAG "nfc_app"

#define RX_FRAME_PRFX "FF55AA55AA"

#define RX_FRAME_PRFX_LEN (10)
#define RX_FRAME_DATA_LEN (32)

#define RX_FRAME_LEN (RX_FRAME_PRFX_LEN + RX_FRAME_DATA_LEN)
#define TX_FRAME_LEN (10)

static uint8_t rx_data[RX_FRAME_LEN + 1] = {0x00};
static uint8_t tx_data[TX_FRAME_LEN + 1] = {0x00, 0xA4, 0x04, 0x00, 0x05};

static nfc_app_mode_t nfc_app_mode = NFC_APP_MODE_IDX_OFF;

static int char2int(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else {
        return 0;
    }
}

static void str2byte(const char *str, char *byte)
{
    for (; str[0] && str[1]; str += 2) {
        *(byte++) = char2int(str[0]) * 16 + char2int(str[1]);
    }
}

static void nfc_app_task_handle(void *pvParameter)
{
    nfc_target nt;
    nfc_device *pnd;
    nfc_context *context;
    nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106
    };
    portTickType xLastWakeTime;

    str2byte(RX_FRAME_PRFX, (char *)tx_data + 5);

    nfc_init(&context);
    if (context == NULL) {
        ESP_LOGE(TAG, "failed to init libnfc");
        goto err;
    }

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            NFC_APP_RUN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        xLastWakeTime = xTaskGetTickCount();

        while ((pnd = nfc_open(context, "pn532_uart:uart1:115200")) == NULL) {
            ESP_LOGE(TAG, "device hard reset");
            pn532_setpin_reset(0);
            vTaskDelay(100 / portTICK_RATE_MS);
            pn532_setpin_reset(1);
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        int res = 0;
        if (nfc_initiator_init(pnd) >= 0) {
            if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) >= 0) {
                if ((res = nfc_initiator_transceive_bytes(pnd, tx_data, TX_FRAME_LEN, rx_data, RX_FRAME_LEN, -1)) >= 0) {
                    rx_data[res] = 0x00;
                } else {
                    ESP_LOGW(TAG, "failed to transceive bytes");
                }
            } else {
                ESP_LOGI(TAG, "available memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_32BIT));
            }
        } else {
            ESP_LOGE(TAG, "failed to init device");
        }

        nfc_close(pnd);

        if (res > 0) {
            if (strstr((char *)rx_data, RX_FRAME_PRFX) != NULL &&
                strlen((char *)rx_data + RX_FRAME_PRFX_LEN) == RX_FRAME_DATA_LEN) {
                ESP_LOGW(TAG, "token: %32s", rx_data + RX_FRAME_PRFX_LEN);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                audio_player_play_file(MP3_FILE_IDX_NOTIFY);
#endif
                http_app_verify_token((char *)rx_data + RX_FRAME_PRFX_LEN);
            } else {
                ESP_LOGW(TAG, "unexpected frame");
            }
        }

        vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_RATE_MS);
    }

err:
    nfc_exit(context);

    ESP_LOGE(TAG, "unrecoverable error");

    os_pwr_reset_wait(OS_PWR_DUMMY_BIT);

    vTaskDelete(NULL);
}

void nfc_app_set_mode(nfc_app_mode_t idx)
{
    nfc_app_mode = idx;

    if (nfc_app_mode == NFC_APP_MODE_IDX_ON) {
        pn532_setpin_reset(1);
        vTaskDelay(100 / portTICK_RATE_MS);
        xEventGroupSetBits(user_event_group, NFC_APP_RUN_BIT);
    } else {
        xEventGroupClearBits(user_event_group, NFC_APP_RUN_BIT);
        pn532_setpin_reset(0);
    }
}

nfc_app_mode_t nfc_app_get_mode(void)
{
    return nfc_app_mode;
}

void nfc_app_init(void)
{
    xTaskCreatePinnedToCore(nfc_app_task_handle, "nfcAppT", 5120, NULL, 5, NULL, 0);
}
