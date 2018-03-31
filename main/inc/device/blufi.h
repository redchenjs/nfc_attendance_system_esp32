/*
 * blufi.h
 *
 *  Created on: 2018-03-30 16:40
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef DEVICE_BLUFI_C_
#define DEVICE_BLUFI_C_

enum blufi_status_table {
    BLUFI_DISCONNECTED = 0,
    BLUFI_CONNECTED = 1
};

#include <stdint.h>

extern uint8_t blufi0_status;

extern void blufi0_init(void);
extern void blufi0_deinit(void);

#endif /* DEVICE_BLUFI_C_ */
