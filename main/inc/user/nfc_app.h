/*
 * nfc_app.h
 *
 *  Created on: 2018-02-13 21:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_NFC_APP_H_
#define INC_USER_NFC_APP_H_

typedef enum {
    NFC_APP_MODE_IDX_OFF = 0x00,
    NFC_APP_MODE_IDX_ON  = 0x01
} nfc_app_mode_t;

extern void nfc_app_set_mode(nfc_app_mode_t idx);
extern nfc_app_mode_t nfc_app_get_mode(void);

extern void nfc_app_init(void);

#endif /* INC_USER_NFC_APP_H_ */
