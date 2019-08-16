/*
 * http_token.h
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP_TOKEN_H_
#define INC_USER_HTTP_TOKEN_H_

#include "esp_http_client.h"

extern esp_err_t http_token_event_handler(esp_http_client_event_t *evt);
extern void http_token_prepare_data(char *buf, int len);
extern void http_token_verify(char *token);

#endif /* INC_USER_HTTP_TOKEN_H_ */
