/*
 * oled_display.h
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_OLED_DISPLAY_H_
#define INC_TASKS_OLED_DISPLAY_H_

enum oled_display_status_table {
    OLED_DISPLAY_RUNNING = 0,
    OLED_DISPLAY_RELOAD  = 1,
    OLED_DISPLAY_STOPPED = 2
};

#include <stdint.h>

extern void oled_display_show_image(uint8_t filename_index);

extern void oled_display_task(void *pvParameter);

#endif /* INC_TASKS_OLED_DISPLAY_H_ */
