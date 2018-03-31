/*
 * blufi_security.h
 *
 *  Created on: 2018-03-30 15:25
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_BLUFI_SECURITY_H_
#define INC_TASKS_BLUFI_SECURITY_H_

#include <stdint.h>

extern void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free);
extern int blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
extern int blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
extern uint16_t blufi_crc_checksum(uint8_t iv8, uint8_t *data, int len);

extern int blufi_security_init(void);
extern void blufi_security_deinit(void);

#endif /* INC_TASKS_BLUFI_SECURITY_H_ */
