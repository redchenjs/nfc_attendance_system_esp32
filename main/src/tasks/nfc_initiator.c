/*
 * nfc_initiator.c
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "nfc/nfc.h"
#include "buses/emdev.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "device/wifi.h"

#include "tasks/mp3_player.h"
#include "tasks/sntp_client.h"
#include "tasks/oled_display.h"
#include "tasks/led_indicator.h"
#include "tasks/token_verifier.h"

#define TAG "nfc_initiator"

#define RX_FRAME_PRFX "f222222222"

#define RX_FRAME_PRFX_LEN (10)
#define RX_FRAME_DATA_LEN (32)

#define RX_FRAME_LEN (RX_FRAME_PRFX_LEN + RX_FRAME_DATA_LEN)
#define TX_FRAME_LEN (10)

uint8_t abtRx[RX_FRAME_LEN + 1] = {0};
uint8_t abtTx[TX_FRAME_LEN + 1] = {0x00,0xA4,0x04,0x00,0x05,0xF2,0x22,0x22,0x22,0x22};

xTaskHandle nfc_initiator_task_handle;

void nfc_initiator_set_mode(uint8_t mode)
{
    if (mode) {
        vTaskResume(nfc_initiator_task_handle);
    } else {
        vTaskSuspend(nfc_initiator_task_handle);
    }
}

void nfc_initiator_task(void *pvParameter)
{
    int res;
    nfc_target nt;
    nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106
    };

    nfc_initiator_set_mode(0);
    oled_display_show_image(3);
    led_indicator_set_mode(1);

    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(wifi0_event_group, WIFI0_CONNECTED_BIT, false, true, 0);
        if ((uxBits & WIFI0_CONNECTED_BIT) != WIFI0_CONNECTED_BIT) {
            oled_display_show_image(0);
            led_indicator_set_mode(7);
            nfc_initiator_set_mode(0);
        }
        nfc_device *pnd = nfc_open(&emdev);
        if (pnd == NULL) {
            break;
        }
        if(nfc_initiator_init(pnd) >= 0) {
            if (nfc_initiator_select_passive_target(pnd, nm, NULL, 0, &nt) >= 0) {
                if ((res = nfc_initiator_transceive_bytes(pnd, abtTx, TX_FRAME_LEN, abtRx, RX_FRAME_LEN, -1)) >= 0) {
                    abtRx[res] = 0;
                    if (strstr((char *)abtRx, RX_FRAME_PRFX) != NULL) {
                        if (strlen((char *)(abtRx + RX_FRAME_PRFX_LEN)) == RX_FRAME_DATA_LEN) {
                            oled_display_show_image(1);
                            mp3_player_play_file(0);
                            token_verifier_verify_token((char *)(abtRx + RX_FRAME_PRFX_LEN));
                            led_indicator_set_mode(5);
                            nfc_initiator_set_mode(0);
                        } else {
                            ESP_LOGW(TAG, "invalid frame data");
                        }
                    } else {
                        ESP_LOGW(TAG, "invalid frame prefix");
                    }
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
        nfc_close(pnd);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    ESP_LOGE(TAG, "could not open nfc device, rebooting...");
    oled_display_show_image(4);
    vTaskDelay(5000 / portTICK_RATE_MS);
    esp_restart();
}
