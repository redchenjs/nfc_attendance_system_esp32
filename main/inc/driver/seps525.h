/*
 * seps525.h
 *
 *  Created on: 2017年4月22日
 *      Author: redchenjs
 */

#ifndef INC_DRIVERS_SEPS525_H_
#define INC_DRIVERS_SEPS525_H_

#include "driver/spi_master.h"

#define Max_Column  0x9F            // 160-1
#define Max_Row     0x7F            // 128-1

extern void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g);
extern void Fill_RAM(unsigned char a, unsigned char b);
extern void Fill_Block(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);
extern void Checkerboard();
extern void Rainbow();
extern void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f);
extern void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c, unsigned char d, unsigned char e);
extern void Show_64k_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
extern void Show_256k_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d);
extern void Up_Scroll();
extern void Down_Scroll();
extern void First_Screen(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
extern void Second_Screen(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h);
extern void Screen_Saver_Control(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h, unsigned char i, unsigned char j, unsigned char k, unsigned char l);
extern void Fade_In();
extern void Fade_Out();
extern void Sleep(unsigned char a);
extern void Set_Gray_Scale_Table();
extern void seps525_init(spi_device_handle_t spi);
extern void seps_spi_pre_transfer_callback(spi_transaction_t *t);

#endif /* INC_DRIVERS_SEPS525_H_ */
