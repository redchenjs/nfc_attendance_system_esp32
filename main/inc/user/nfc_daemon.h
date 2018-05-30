/*
 * nfc_daemon.h
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_NFC_DAEMON_H_
#define INC_USER_NFC_DAEMON_H_

#include <stdint.h>

extern void nfc_daemon(void *pvParameter);
extern void nfc_set_mode(uint8_t mode);

#endif /* INC_USER_NFC_DAEMON_H_ */
