/*
 * token_verify.h
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_TOKEN_VERIFY_H_
#define INC_TASKS_TOKEN_VERIFY_H_

#include "tasks/http2_client.h"

extern int token_verify_parse_data(struct http2c_handle *handle, const char *data, size_t len, int flags);
extern int token_verify_prepare_data(struct http2c_handle *handle, char *buf, size_t length, uint32_t *data_flags);

extern void token_verify(char *token);

#endif /* INC_TASKS_TOKEN_VERIFY_H_ */
