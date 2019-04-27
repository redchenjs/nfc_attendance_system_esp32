/*
 * nfc.h
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_NFC_H_
#define INC_USER_NFC_H_

#include <stdint.h>

extern void nfc_task(void *pvParameter);
extern void nfc_set_mode(uint8_t mode);

#endif /* INC_USER_NFC_H_ */
