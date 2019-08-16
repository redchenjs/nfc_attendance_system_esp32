/*
 * http_ota.h
 *
 *  Created on: 2018-04-06 15:12
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_USER_HTTP_OTA_H_
#define INC_USER_HTTP_OTA_H_

#include "esp_http_client.h"

extern esp_err_t http_ota_event_handler(esp_http_client_event_t *evt);
extern void http_ota_prepare_data(char *buf, int len);
extern void http_ota_update(void);

#endif /* INC_USER_HTTP_OTA_H_ */
