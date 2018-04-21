/*
 * nfc_daemon.c
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "buses/emdev.h"

#include "system/event.h"
#include "tasks/gui_daemon.h"
#include "tasks/ntp_daemon.h"
#include "tasks/nfc_daemon.h"
#include "tasks/led_daemon.h"
#include "tasks/audio_daemon.h"
#include "tasks/token_verify.h"

#define TAG "nfc"

#define RX_FRAME_PRFX "f222222222"

#define RX_FRAME_PRFX_LEN (10)
#define RX_FRAME_DATA_LEN (32)

#define RX_FRAME_LEN (RX_FRAME_PRFX_LEN + RX_FRAME_DATA_LEN)
#define TX_FRAME_LEN (10)

static uint8_t abtRx[RX_FRAME_LEN + 1] = {0};
static uint8_t abtTx[TX_FRAME_LEN + 1] = {0x00,0xA4,0x04,0x00,0x05,0xF2,0x22,0x22,0x22,0x22};

void nfc_daemon(void *pvParameter)
{
    nfc_target nt;
    nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106
    };

    emdev_init();

    while (1) {
        xEventGroupWaitBits(
            daemon_event_group,
            NFC_DAEMON_READY_BIT,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY
        );
        // Open NFC device
        nfc_device *pnd = nfc_open(&emdev);
        if (pnd == NULL) {
            break;
        }
        // Transceive some bytes if target available
        int res = 0;
        if (nfc_initiator_init(pnd) >= 0) {
            if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) >= 0) {
                if ((res = nfc_initiator_transceive_bytes(pnd, abtTx, TX_FRAME_LEN, abtRx, RX_FRAME_LEN, -1)) >= 0) {
                    abtRx[res] = 0;
                } else {
                    ESP_LOGW(TAG, "not a valid target");
                }
                nfc_initiator_deselect_target(pnd);
            } else {
                ESP_LOGI(TAG, "waiting for target");
            }
        } else {
            ESP_LOGE(TAG, "could not init nfc device");
        }
        // Close NFC device
        nfc_close(pnd);
        // Match received bytes and verify the token if available
        if (res > 0) {
            if (strstr((char *)abtRx, RX_FRAME_PRFX) != NULL) {
                if (strlen((char *)(abtRx + RX_FRAME_PRFX_LEN)) == RX_FRAME_DATA_LEN) {
                    ESP_LOGW(TAG, "total free mem %u", heap_caps_get_free_size(MALLOC_CAP_32BIT));
                    audio_play_file(0);
                    token_verify((char *)(abtRx + RX_FRAME_PRFX_LEN));
                } else {
                    ESP_LOGW(TAG, "invalid frame data");
                }
            } else {
                ESP_LOGW(TAG, "invalid frame prefix");
            }
        }
        // Task Delay
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    ESP_LOGE(TAG, "could not open nfc device, rebooting...");
    gui_show_image(4);
    vTaskDelay(2000 / portTICK_RATE_MS);
    esp_restart();
}

void nfc_set_mode(uint8_t mode)
{
    if (mode != 0) {
        xEventGroupSetBits(daemon_event_group, NFC_DAEMON_READY_BIT);
    } else {
        xEventGroupClearBits(daemon_event_group, NFC_DAEMON_READY_BIT);
    }
}
