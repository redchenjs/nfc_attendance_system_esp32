/*
 * gui_daemon.h
 *
 *  Created on: 2018-02-13 22:57
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef INC_TASKS_GUI_DAEMON_H_
#define INC_TASKS_GUI_DAEMON_H_

#include <stdint.h>

#if defined(CONFIG_SCREEN_PANEL_SSD1331)
// ani0.gif
extern const uint8_t ani0_96x64_gif_ptr[] asm("_binary_ani0_96x64_gif_start");
extern const uint8_t ani0_96x64_gif_end[] asm("_binary_ani0_96x64_gif_end");
// ani1.gif
extern const uint8_t ani1_96x64_gif_ptr[] asm("_binary_ani1_96x64_gif_start");
extern const uint8_t ani1_96x64_gif_end[] asm("_binary_ani1_96x64_gif_end");
// ani2.gif
extern const uint8_t ani2_96x64_gif_ptr[] asm("_binary_ani2_96x64_gif_start");
extern const uint8_t ani2_96x64_gif_end[] asm("_binary_ani2_96x64_gif_end");
// ani3.gif
extern const uint8_t ani3_96x64_gif_ptr[] asm("_binary_ani3_96x64_gif_start");
extern const uint8_t ani3_96x64_gif_end[] asm("_binary_ani3_96x64_gif_end");
// ani4.gif
extern const uint8_t ani4_96x64_gif_ptr[] asm("_binary_ani4_96x64_gif_start");
extern const uint8_t ani4_96x64_gif_end[] asm("_binary_ani4_96x64_gif_end");
// ani5.gif
extern const uint8_t ani5_96x64_gif_ptr[] asm("_binary_ani5_96x64_gif_start");
extern const uint8_t ani5_96x64_gif_end[] asm("_binary_ani5_96x64_gif_end");
// ani6.gif
extern const uint8_t ani6_96x64_gif_ptr[] asm("_binary_ani6_96x64_gif_start");
extern const uint8_t ani6_96x64_gif_end[] asm("_binary_ani6_96x64_gif_end");
// ani7.gif
extern const uint8_t ani7_96x64_gif_ptr[] asm("_binary_ani7_96x64_gif_start");
extern const uint8_t ani7_96x64_gif_end[] asm("_binary_ani7_96x64_gif_end");
#elif defined(CONFIG_SCREEN_PANEL_ST7735)
// ani0.gif
extern const uint8_t ani0_160x80_gif_ptr[] asm("_binary_ani0_160x80_gif_start");
extern const uint8_t ani0_160x80_gif_end[] asm("_binary_ani0_160x80_gif_end");
// ani1.gif
extern const uint8_t ani1_160x80_gif_ptr[] asm("_binary_ani1_160x80_gif_start");
extern const uint8_t ani1_160x80_gif_end[] asm("_binary_ani1_160x80_gif_end");
// ani2.gif
extern const uint8_t ani2_160x80_gif_ptr[] asm("_binary_ani2_160x80_gif_start");
extern const uint8_t ani2_160x80_gif_end[] asm("_binary_ani2_160x80_gif_end");
// ani3.gif
extern const uint8_t ani3_160x80_gif_ptr[] asm("_binary_ani3_160x80_gif_start");
extern const uint8_t ani3_160x80_gif_end[] asm("_binary_ani3_160x80_gif_end");
// ani4.gif
extern const uint8_t ani4_160x80_gif_ptr[] asm("_binary_ani4_160x80_gif_start");
extern const uint8_t ani4_160x80_gif_end[] asm("_binary_ani4_160x80_gif_end");
// ani5.gif
extern const uint8_t ani5_160x80_gif_ptr[] asm("_binary_ani5_160x80_gif_start");
extern const uint8_t ani5_160x80_gif_end[] asm("_binary_ani5_160x80_gif_end");
// ani6.gif
extern const uint8_t ani6_160x80_gif_ptr[] asm("_binary_ani6_160x80_gif_start");
extern const uint8_t ani6_160x80_gif_end[] asm("_binary_ani6_160x80_gif_end");
// ani7.gif
extern const uint8_t ani7_160x80_gif_ptr[] asm("_binary_ani7_160x80_gif_start");
extern const uint8_t ani7_160x80_gif_end[] asm("_binary_ani7_160x80_gif_end");
#endif

extern void gui_daemon_show_image(uint8_t filename_index);
extern void gui_daemon_task(void *pvParameter);

#endif /* INC_TASKS_GUI_DAEMON_H_ */
