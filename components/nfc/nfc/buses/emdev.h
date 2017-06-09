#ifndef __EMDEV_H__
#define __EMDEV_H__

#include "nfc-internal.h"

#define CFG_UART_BUFSIZE	512

typedef struct _uart_buffer_t
{
	uint8_t ep_dir;
	volatile uint16_t len;
	volatile uint16_t wr_ptr;
	volatile uint16_t rd_ptr;
	uint8_t buf[CFG_UART_BUFSIZE];
}uart_buffer_t;

typedef struct _uart_pcb_t
{
	uint8_t status;
	uint8_t pending_tx_data;
	uart_buffer_t rxfifo;
}uart_pcb_t;

/*
void uartInit(void);
int uartSend(uint8_t *buffer, uint32_t length, int timeout);
void uartSendByte(uint8_t ch);
uint8_t uartRxBufferRead(void);
int uartRxBufferReadArray(uint8_t *rx, uint32_t len, int timeout);
uint8_t uartRxBufferDataPending(void);
uart_pcb_t *uartGetPCB(void);
*/

void uartRxBufferWrite(uint8_t data);
void emdev_init(nfc_emdev *emdev);


#endif
