#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "soc/uart_struct.h"

#include "emdev.h"

static const char *TAG = "uart_events";

/**
 * This example shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - port: UART1
 * - rx buffer: on
 * - tx buffer: on
 * - flow control: off
 * - event queue: off
 * - pin assignment: txd(default), rxd(default)
 */

#define LOG_UART_NUM UART_NUM_0

#define EX_UART_NUM UART_NUM_1
#define EX_UART_TXD  (4)
#define EX_UART_RXD  (0)

#define BUF_SIZE (1024)

/* Configure the UART events example */
void uart_event_init()
{
    uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
       .rx_flow_ctrl_thresh = 122,
    };
    //Set UART parameters
    uart_param_config(EX_UART_NUM, &uart_config);
    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);

    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, EX_UART_TXD, EX_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int uartSend(uint8_t *buffer, uint32_t length, int timeout)
{
    uart_write_bytes(EX_UART_NUM, (const char*)buffer, length);
    uart_wait_tx_done(EX_UART_NUM, timeout / portTICK_RATE_MS);
	return 0;
}

int uartRxBufferReadArray(uint8_t *rx, uint32_t len, int timeout)
{
    int err;
    err = uart_read_bytes(EX_UART_NUM, rx, len, timeout / portTICK_RATE_MS);
    if (err == -1) {
      printf("\r\n receive drv timeout\r\n");
      return -1;
    }
  return 0;
}

void mopen(void)
{
//    printf("mopen\r\n");
}

int mreceive(uint8_t *pbtRx, const size_t szRx, int timeout)
{
  return uartRxBufferReadArray(pbtRx, szRx, timeout);
}
//const 
int msend(const uint8_t *pbtTx, const size_t szTx, int timeout)
{
  return uartSend((uint8_t *)pbtTx, szTx, timeout);
}

void mclose(void)
{
//  printf("mclose\r\n");
}

void emdev_init(nfc_emdev *emdev)
{
  uart_event_init();
  emdev->open = mopen;
  emdev->receive = mreceive;
  emdev->send = msend;
  emdev->close = mclose;
}


