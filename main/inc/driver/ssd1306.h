#ifndef SSD1306_H_
#define SSD1306_H_

enum ssd1306_mode {
    NORMAL    = 1, // Normal display
    INVERSE   = 0  // Inverse display
};

#define SSD1306_CMD    0
#define SSD1306_DATA   1

#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT   64

extern void ssd1306_clear_screen(unsigned char chFill);
extern void ssd1306_refresh_gram(void);
extern void ssd1306_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned char chPoint);
extern void ssd1306_fill_screen(unsigned char chXpos1, unsigned char chYpos1, unsigned char chXpos2, unsigned char chYpos2, unsigned char chDot);
extern void ssd1306_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chSize, unsigned char chMode);
extern void ssd1306_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen,unsigned char chSize);
extern void ssd1306_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chSize, unsigned char chMode);
extern void ssd1306_draw_1612char(unsigned char chXpos, unsigned char chYpos, unsigned char chChar);
extern void ssd1306_draw_3216char(unsigned char chXpos, unsigned char chYpos, unsigned char chChar);
extern void ssd1306_draw_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight);

extern void ssd1306_init(void);

#endif
