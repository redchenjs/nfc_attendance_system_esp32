/*
 * ota_update.h
 *
 *  Created on: 2018-04-06 15:12
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_OTA_UPDATE_H_
#define INC_TASKS_OTA_UPDATE_H_

#include "tasks/http2_client.h"

extern int ota_update_parse_data(struct http2c_handle *handle, const char *data, size_t len, int flags);
extern int ota_update_prepare_data(struct http2c_handle *handle, char *buf, size_t length, uint32_t *data_flags);

extern void ota_update(void);

#endif /* INC_TASKS_OTA_UPDATE_H_ */
