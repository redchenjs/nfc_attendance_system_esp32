/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tartière
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/**
 * @file uart.c
 * @brief UART driver
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

#include "uart.h"

#include <nfc/nfc.h>
#include "nfc-internal.h"

#define LOG_GROUP    NFC_LOG_GROUP_COM
#define LOG_CATEGORY "libnfc.bus.uart"

serial_port
uart_open(const char *pcPortName)
{
  serial_port sp = (serial_port)UART_NUM;

  sscanf(pcPortName, "uart%1u", (unsigned int *)&sp);
  if ((uart_port_t)sp >= UART_NUM_MAX) {
    return INVALID_SERIAL_PORT;
  }

  return sp;
}

void
uart_set_speed(serial_port sp, const uint32_t uiPortSpeed)
{
  log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG, "Serial port speed requested to be set to %d baud.", uiPortSpeed);

  if (uart_set_baudrate((uart_port_t)sp, uiPortSpeed)) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "%s", "Unable to apply new speed settings.");
  }
}

uint32_t
uart_get_speed(serial_port sp)
{
  uint32_t uiPortSpeed = 0;
  uart_get_baudrate((uart_port_t)sp, &uiPortSpeed);

  return uiPortSpeed;
}

void
uart_close(const serial_port sp) {}

int
uart_receive(serial_port sp, uint8_t *pbtRx, const size_t szRx, void *abort_p, int timeout)
{
  if (szRx == uart_read_bytes((uart_port_t)sp, pbtRx, szRx, timeout / portTICK_RATE_MS)) {
    LOG_HEX(LOG_GROUP, "RX", pbtRx, szRx);
    return NFC_SUCCESS;
  } else {
    return NFC_EIO;
  }
}

int
uart_send(serial_port sp, const uint8_t *pbtTx, const size_t szTx, int timeout)
{
  LOG_HEX(LOG_GROUP, "TX", pbtTx, szTx);
  if (szTx == uart_write_bytes((uart_port_t)sp, (const char *)pbtTx, szTx)) {
    return NFC_SUCCESS;
  } else {
    return NFC_EIO;
  }
}

char **
uart_list_ports(void)
{
  char **res = calloc(UART_NUM_MAX + 1, sizeof(char *));
  if (!res) {
    perror("malloc");
    return res;
  }

  for (int i = 0; i < UART_NUM_MAX; i++) {
    if (!(res[i] = calloc(6, sizeof(char)))) {
      perror("malloc");
      return res;
    }
    sprintf(res[i], "uart%1u", i);
  }

  return res;
}
