#ifndef FONTS_H
#define FONTS_H

enum font_size {
    FONT_1206  = 12,
    FONT_1608  = 16,
    FONT_1612  = 18,
    FONT_3216  = 32
};

extern const unsigned char c_chFont1206[95][12];
extern const unsigned char c_chFont1608[95][16];
extern const unsigned char c_chFont1612[11][32];
extern const unsigned char c_chFont3216[11][64];
extern const unsigned char c_chBmp4016[96];
extern const unsigned char c_chSingal816[16];
extern const unsigned char c_chMsg816[16];
extern const unsigned char c_chBluetooth88[8];
extern const unsigned char c_chBat816[16];
extern const unsigned char c_chGPRS88[8];
extern const unsigned char c_chAlarm88[8];

#endif
