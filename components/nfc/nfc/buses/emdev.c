/*
 * emdev.c
 *
 *  Created on: 2018-02-10 16:36
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "nfc-internal.h"

#include "esp_log.h"
#include "driver/uart.h"
#include "device/uart.h"

nfc_emdev emdev;

#define TAG "libnfc"

void mopen(void) {}

int mreceive(uint8_t *pbtRx, const size_t szRx, int timeout)
{
    int err;
    err = uart_read_bytes(EX_UART_NUM, pbtRx, szRx, timeout / portTICK_RATE_MS);
    if (err == -1) {
        ESP_LOGE(TAG, "uart receive timeout\n");
        return -1;
    }
    return 0;
}

int msend(const uint8_t *pbtTx, const size_t szTx, int timeout)
{
    uart_write_bytes(EX_UART_NUM, (const char*)pbtTx, szTx);
    return 0;
}

void mclose(void) {}

void emdev_init(void)
{
    emdev.open    = mopen;
    emdev.receive = mreceive;
    emdev.send    = msend;
    emdev.close   = mclose;
}
