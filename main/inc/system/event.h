/*
 * event.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef SYSTEM_EVENT_C_
#define SYSTEM_EVENT_C_

#include "esp_err.h"
#include "esp_event_loop.h"

extern esp_err_t system_event_handler(void *ctx, system_event_t *event);

#endif /* SYSTEM_EVENT_C_ */
