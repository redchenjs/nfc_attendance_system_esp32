/*
 * token_verifier.h
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_TOKEN_VERIFIER_H_
#define INC_TASKS_TOKEN_VERIFIER_H_

#include <stdint.h>

// cert0.pem
extern const uint8_t cert0_pem_ptr[] asm("_binary_cert0_pem_start");
extern const uint8_t cert0_pem_end[] asm("_binary_cert0_pem_end");

extern void token_verifier_verify_token(char *token);
extern void token_verifier_task(void *pvParameter);

#endif /* INC_TASKS_TOKEN_VERIFIER_H_ */
