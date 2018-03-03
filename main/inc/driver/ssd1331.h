/*
 * ssd1331.c
 *
 *  Created on: 2018-02-10 15:55
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef SSD1331_H_
#define SSD1331_H_

#include <stdint.h>

extern uint8_t *ssd1331_gram_ptr;
extern void ssd1331_gram_refresh(void);

extern void ssd1331_write_byte(unsigned char chData, unsigned char chCmd);

extern void ssd1331_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned int hwColor);
extern void ssd1331_draw_line(unsigned char chXpos0, unsigned char chYpos0, unsigned char chXpos1, unsigned char chYpos1, unsigned int hwColor);
extern void ssd1331_draw_v_line(unsigned char chXpos, unsigned char chYpos, unsigned char chHeight, unsigned int hwColor);
extern void ssd1331_draw_h_line(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned int hwColor);
extern void ssd1331_draw_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor);
extern void ssd1331_draw_circle(unsigned char chXpos, unsigned char chYpos, unsigned char chRadius, unsigned int hwColor);

extern void ssd1331_draw_mono_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1331_draw_64k_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight);

extern void ssd1331_fill_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor);
extern void ssd1331_fill_gram(unsigned int hwColor);

extern void ssd1331_clear_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight);
extern void ssd1331_clear_gram(void);

extern void ssd1331_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1331_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1331_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);

extern void ssd1331_continuous_scrolling(unsigned char chYpos, unsigned char chHeight, unsigned char chDirection, unsigned char chInterval);
extern void ssd1331_deactivate_scrolling(void);

extern void ssd1331_show_checkerboard(void);
extern void ssd1331_show_rainbow(void);

extern void ssd1331_set_gray_scale_table(void);

extern void ssd1331_init(void);

#endif
