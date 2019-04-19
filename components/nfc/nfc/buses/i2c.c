/*-
 * Free/Libre Near Field Communication (NFC) library
 *
 * Libnfc historical contributors:
 * Copyright (C) 2009      Roel Verdult
 * Copyright (C) 2009-2013 Romuald Conty
 * Copyright (C) 2010-2012 Romain Tarti?re
 * Copyright (C) 2010-2013 Philippe Teuwen
 * Copyright (C) 2012-2013 Ludovic Rousseau
 * See AUTHORS file for a more comprehensive list of contributors.
 * Additional contributors of this file:
 * Copyright (C) 2013      Laurent Latil
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
 * @file i2c.c
 * @brief I2C driver (implemented / tested for Linux only currently)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H
#include "i2c.h"

#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <nfc/nfc.h>
#include "nfc-internal.h"

#include "esp_log.h"
#include "driver/i2c.h"

#include "device/i2c.h"

#define LOG_GROUP    NFC_LOG_GROUP_COM
#define LOG_CATEGORY "libnfc.bus.i2c"

static uint8_t i2c_dev_addr = 0x00;

void
i2c_open(i2c_port_t port, uint8_t addr)
{
  i2c_dev_addr = addr;
}

void
i2c_close(i2c_port_t port)
{

}

/**
 * @brief Read a frame from the I2C device and copy data to \a pbtRx
 *
 * @param pbtRx pointer on buffer used to store data
 * @param szRx length of the buffer
 * @return length (in bytes) of read data, or driver error code  (negative value)
 */
ssize_t
i2c_read(i2c_port_t port, uint8_t *pbtRx, const size_t szRx)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, i2c_dev_addr << 1 | I2C_MASTER_READ, 1);
  if (szRx > 1) {
      i2c_master_read(cmd, pbtRx, szRx - 1, 0);
  }
  i2c_master_read_byte(cmd, pbtRx + szRx - 1, 1);
  i2c_master_stop(cmd);
  int res = i2c_master_cmd_begin(port, cmd, 500 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  LOG_HEX(LOG_GROUP, "RX", pbtRx, szRx);
  if (res == ESP_OK) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG,
            "read %d bytes successfully.", szRx);
    return szRx;
  } else if (res == ESP_ERR_TIMEOUT) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "Error: read timeout.");
    return NFC_ETIMEOUT;
  } else {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "Error: read error.");
    return NFC_EIO;
  }
}

/**
 * @brief Write a frame to I2C device containing \a pbtTx content
 *
 * @param pbtTx pointer on buffer containing data
 * @param szTx length of the buffer
 * @return NFC_SUCCESS on success, otherwise driver error code
 */
int
i2c_write(i2c_port_t port, const uint8_t *pbtTx, const size_t szTx)
{
  LOG_HEX(LOG_GROUP, "TX", pbtTx, szTx);

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, i2c_dev_addr << 1 | I2C_MASTER_WRITE, 1);
  i2c_master_write(cmd, (uint8_t *)pbtTx, szTx, 1);
  i2c_master_stop(cmd);
  int res = i2c_master_cmd_begin(port, cmd, 500 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  if (res == ESP_OK) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_DEBUG,
            "wrote %d bytes successfully.", (int)szTx);
    return NFC_SUCCESS;
  } else if (res == ESP_ERR_TIMEOUT) {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "Error: write timeout.");
    return NFC_ETIMEOUT;
  } else {
    log_put(LOG_GROUP, LOG_CATEGORY, NFC_LOG_PRIORITY_ERROR, "Error: write error.");
    return NFC_EIO;
  }
}
