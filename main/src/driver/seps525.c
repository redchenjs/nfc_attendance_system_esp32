#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#include "inc/driver/seps525.h"
/*
 * seps525.c
 *
 *  Created on: 2017-4-22
 *      Author: redchenjs
 */

// 4-Wire SPI
#define SEPS525_PIN_NUM_MISO 25
#define SEPS525_PIN_NUM_MOSI 23
#define SEPS525_PIN_NUM_CLK  19
#define SEPS525_PIN_NUM_CS   22

#define SEPS525_PIN_NUM_DC   21
#define SEPS525_PIN_NUM_RST  18

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} seps_init_cmd_t;

spi_device_handle_t spi_device = NULL;

#define SEPS525_DELAY_US(X)     vTaskDelay(X / 10 / portTICK_RATE_MS)

//Send a command to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
void seps_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//Send data to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
void seps_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void seps_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(SEPS525_PIN_NUM_DC, dc);
}

void Write_Data(unsigned char Data)
{
    seps_data(spi_device, &Data, 1);
}

void Write_Command(unsigned char Data)
{
    seps_cmd(spi_device, Data);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Read_Index(unsigned char d)
{
    Write_Command(0x00);            // Read Index Address of Registers (INDEX)
//  Read_Data(d);               //   Default => 0x00
}


void Set_Read_Status(unsigned char d)
{
    Write_Command(0x01);            // Read Internal Status (STATUS_RD)
//  Read_Data(d);               //   Default => 0xC0
}


void Set_Clock_Control(unsigned char d)
{
    Write_Command(0x02);            // Set Oscillator Control (OSC_CTL)
    Write_Data(d);              //   Default => 0xC0
                        //     EXPORT1 Pin at Low Level
                        //     Oscillator operates with internal resister.
                        //     Clock Off
}


void Set_Display_Clock(unsigned char d)
{
    Write_Command(0x03);            // Set Display Frequency Divide Ration / Oscillator Frequency (CLOCK_DIV)
    Write_Data(d);              //   Default => 0x30 (90 Hz / Divide Ration = 1)
                        //     D[3:0] => Display Frequency Divide Ration
                        //     D[7:4] => Oscillator Frequency
}


void Set_Power_Save(unsigned char d)
{
    Write_Command(0x04);            // Set Power Save Mode (REDUCE_CURRENT)
    Write_Data(d);              //   Default => 0x00
                        //     Normal Driving Current
                        //     Disable Oscillator Power Down
                        //     Disable Power Save Mode
}


void Software_Reset(unsigned char d)
{
    Write_Command(0x05);            // Software Reset (SOFT_RST)
    Write_Data(d);              //   Default => 0x00
                        //     0x00 => Normal Mode
                        //     0x01 => All internal register values will be default.
}


void Set_Display_On_Off(unsigned char d)
{
    Write_Command(0x06);            // Set Display On/Off (DISP_ON_OFF)
    Write_Data(d);              //   Default => 0x00
                        //     Scan signal is high level at pre-charge period.
                        //     Display Off
}


void Set_Precharge_Period(unsigned char a, unsigned char b, unsigned char c)
{
    Write_Command(0x08);            // Set Pre-Charge Time of Red (PRECHARGE_TIME_R)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x09);            // Set Pre-Charge Time of Green (PRECHARGE_TIME_G)
    Write_Data(b);              //   Default => 0x00
    Write_Command(0x0A);            // Set Pre-Charge Time of Blue (PRECHARGE_TIME_B)
    Write_Data(c);              //   Default => 0x00
}


void Set_Precharge_Current(unsigned char a, unsigned char b, unsigned char c)
{
    Write_Command(0x0B);            // Set Pre-Charge Current of Red (PRECHARGE_CURRENT_R)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x0C);            // Set Pre-Charge Current of Green (PRECHARGE_CURRENT_G)
    Write_Data(b);              //   Default => 0x00
    Write_Command(0x0D);            // Set Pre-Charge Current of Blue (PRECHARGE_CURRENT_B)
    Write_Data(c);              //   Default => 0x00
}


void Set_Driving_Current(unsigned char a, unsigned char b, unsigned char c)
{
    Write_Command(0x10);            // Set Driving Current of Red (DRIVING_CURRENT_R)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x11);            // Set Driving Current of Green (DRIVING_CURRENT_G)
    Write_Data(b);              //   Default => 0x00
    Write_Command(0x12);            // Set Driving Current of Blue (DRIVING_CURRENT_B)
    Write_Data(c);              //   Default => 0x00
}


void Set_Display_Mode(unsigned char d)
{
    Write_Command(0x13);            // Set Column Data Display Control / Re-Map Format (DISPLAY_MODE_SET)
    Write_Data(d);              //   Default => 0x00
                        //     Color Sequence: R => G => B
                        //     Alternative Gate Pin Configuration
                        //     Scan from G0 to G[N-1]
                        //     Column Address 0 Mapped to S0
                        //     One Screen Mode
                        //     Normal Display
}


void Set_RGB_IF(unsigned char d)
{
    Write_Command(0x14);            // Set Interface Mode (RGB_IF)
    Write_Data(d);              //   Default => 0x11 (MCU Interface Mode)
}


void Set_RGB_POL(unsigned char d)
{
    Write_Command(0x15);            // Set RGB Interface Polarity (RGB_POL)
    Write_Data(d);              //   Default => 0x00
                        //     Enable Polarity as Active Low
                        //     Dot Clock Polarity Sampled as Rising Edge
                        //     Disable Vertical Synchronization Output on VSYNCO Pin
}


void Set_Pixel_Format(unsigned char d)
{
    Write_Command(0x16);            // Set Memory Access Control / Interface Pixel Format (MEMORY_WRITE_MODE)
    Write_Data(d);              //   Default => 0x06
                        //     Enable 18-bit Bus Interface
                        //     262,144 Colors
                        //     Horizontal address counter is increased.
                        //     Vertical address counter is increased.
                        //     The data is continuously written horizontally.
}


void Set_Column_Address(unsigned char a, unsigned char b)
{
    Write_Command(0x17);            // Set Column Address of Start (MX1_ADDR)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x18);            // Set Column Address of End (MX2_ADDR)
    Write_Data(b);              //   Default => 0x9F
}


void Set_Row_Address(unsigned char a, unsigned char b)
{
    Write_Command(0x19);            // Set Row Address of Start (MY1_ADDR)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x1A);            // Set Row Address of End (MY2_ADDR)
    Write_Data(b);              //   Default => 0x7F
}


void Set_Display_Offset(unsigned char a, unsigned char b)
{
    Write_Command(0x20);            // Specify the Horizontal Start Position of a Window for Written in Memory (MEMORY_ACCESSPOINTER X)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x21);            // Specify the Vertical Start Position of a Window for Written in Memory (MEMORY_ACCESSPOINTER Y)
    Write_Data(b);              //   Default => 0x00
}


void Set_Write_RAM()
{
    Write_Command(0x22);            // Internal DDRAM Memory Access (DDRAM_DATA_ACCESS_PORT)
}


void Set_Multiplex_Ratio(unsigned char d)
{
    Write_Command(0x28);            // Display Duty Ratio (DUTY)
    Write_Data(d);              //   Default => 0x7F (1/128 Duty)
}


void Set_Start_Line(unsigned char d)
{
    Write_Command(0x29);            // Set Display Start Line (DSL)
    Write_Data(d);              //   Default => 0x00
}


void Set_IREF(unsigned char d)
{
    Write_Command(0x80);            // Control Reference Voltage Generation (IREF)
    Write_Data(d);              //   Default => 0x00 (Reference Voltage Controlled by External Resister)
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Global Variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define Color_Depth 0x00            // "0x00" => 65,536 Colors Mode
                        // "0x01" => 262,144 Colors Mode
#define Brightness  0xFF

const unsigned char Ascii_1[241][5]={        // Refer to "Times New Roman" Font Database...
                        //   Basic Characters
    {0x00,0x00,0x00,0x00,0x00},     //
    {0x00,0x00,0x4F,0x00,0x00},     //   (  1)  ! - 0x0021 Exclamation Mark
    {0x00,0x07,0x00,0x07,0x00},     //   (  2)  " - 0x0022 Quotation Mark
    {0x14,0x7F,0x14,0x7F,0x14},     //   (  3)  # - 0x0023 Number Sign
    {0x24,0x2A,0x7F,0x2A,0x12},     //   (  4)  $ - 0x0024 Dollar Sign
    {0x23,0x13,0x08,0x64,0x62},     //   (  5)  % - 0x0025 Percent Sign
    {0x36,0x49,0x55,0x22,0x50},     //   (  6)  & - 0x0026 Ampersand
    {0x00,0x05,0x03,0x00,0x00},     //   (  7)  ' - 0x0027 Apostrophe
    {0x00,0x1C,0x22,0x41,0x00},     //   (  8)  ( - 0x0028 Left Parenthesis
    {0x00,0x41,0x22,0x1C,0x00},     //   (  9)  ) - 0x0029 Right Parenthesis
    {0x14,0x08,0x3E,0x08,0x14},     //   ( 10)  * - 0x002A Asterisk
    {0x08,0x08,0x3E,0x08,0x08},     //   ( 11)  + - 0x002B Plus Sign
    {0x00,0x50,0x30,0x00,0x00},     //   ( 12)  , - 0x002C Comma
    {0x08,0x08,0x08,0x08,0x08},     //   ( 13)  - - 0x002D Hyphen-Minus
    {0x00,0x60,0x60,0x00,0x00},     //   ( 14)  . - 0x002E Full Stop
    {0x20,0x10,0x08,0x04,0x02},     //   ( 15)  / - 0x002F Solidus
    {0x3E,0x51,0x49,0x45,0x3E},     //   ( 16)  0 - 0x0030 Digit Zero
    {0x00,0x42,0x7F,0x40,0x00},     //   ( 17)  1 - 0x0031 Digit One
    {0x42,0x61,0x51,0x49,0x46},     //   ( 18)  2 - 0x0032 Digit Two
    {0x21,0x41,0x45,0x4B,0x31},     //   ( 19)  3 - 0x0033 Digit Three
    {0x18,0x14,0x12,0x7F,0x10},     //   ( 20)  4 - 0x0034 Digit Four
    {0x27,0x45,0x45,0x45,0x39},     //   ( 21)  5 - 0x0035 Digit Five
    {0x3C,0x4A,0x49,0x49,0x30},     //   ( 22)  6 - 0x0036 Digit Six
    {0x01,0x71,0x09,0x05,0x03},     //   ( 23)  7 - 0x0037 Digit Seven
    {0x36,0x49,0x49,0x49,0x36},     //   ( 24)  8 - 0x0038 Digit Eight
    {0x06,0x49,0x49,0x29,0x1E},     //   ( 25)  9 - 0x0039 Dight Nine
    {0x00,0x36,0x36,0x00,0x00},     //   ( 26)  : - 0x003A Colon
    {0x00,0x56,0x36,0x00,0x00},     //   ( 27)  ; - 0x003B Semicolon
    {0x08,0x14,0x22,0x41,0x00},     //   ( 28)  < - 0x003C Less-Than Sign
    {0x14,0x14,0x14,0x14,0x14},     //   ( 29)  = - 0x003D Equals Sign
    {0x00,0x41,0x22,0x14,0x08},     //   ( 30)  > - 0x003E Greater-Than Sign
    {0x02,0x01,0x51,0x09,0x06},     //   ( 31)  ? - 0x003F Question Mark
    {0x32,0x49,0x79,0x41,0x3E},     //   ( 32)  @ - 0x0040 Commercial At
    {0x7E,0x11,0x11,0x11,0x7E},     //   ( 33)  A - 0x0041 Latin Capital Letter A
    {0x7F,0x49,0x49,0x49,0x36},     //   ( 34)  B - 0x0042 Latin Capital Letter B
    {0x3E,0x41,0x41,0x41,0x22},     //   ( 35)  C - 0x0043 Latin Capital Letter C
    {0x7F,0x41,0x41,0x22,0x1C},     //   ( 36)  D - 0x0044 Latin Capital Letter D
    {0x7F,0x49,0x49,0x49,0x41},     //   ( 37)  E - 0x0045 Latin Capital Letter E
    {0x7F,0x09,0x09,0x09,0x01},     //   ( 38)  F - 0x0046 Latin Capital Letter F
    {0x3E,0x41,0x49,0x49,0x7A},     //   ( 39)  G - 0x0047 Latin Capital Letter G
    {0x7F,0x08,0x08,0x08,0x7F},     //   ( 40)  H - 0x0048 Latin Capital Letter H
    {0x00,0x41,0x7F,0x41,0x00},     //   ( 41)  I - 0x0049 Latin Capital Letter I
    {0x20,0x40,0x41,0x3F,0x01},     //   ( 42)  J - 0x004A Latin Capital Letter J
    {0x7F,0x08,0x14,0x22,0x41},     //   ( 43)  K - 0x004B Latin Capital Letter K
    {0x7F,0x40,0x40,0x40,0x40},     //   ( 44)  L - 0x004C Latin Capital Letter L
    {0x7F,0x02,0x0C,0x02,0x7F},     //   ( 45)  M - 0x004D Latin Capital Letter M
    {0x7F,0x04,0x08,0x10,0x7F},     //   ( 46)  N - 0x004E Latin Capital Letter N
    {0x3E,0x41,0x41,0x41,0x3E},     //   ( 47)  O - 0x004F Latin Capital Letter O
    {0x7F,0x09,0x09,0x09,0x06},     //   ( 48)  P - 0x0050 Latin Capital Letter P
    {0x3E,0x41,0x51,0x21,0x5E},     //   ( 49)  Q - 0x0051 Latin Capital Letter Q
    {0x7F,0x09,0x19,0x29,0x46},     //   ( 50)  R - 0x0052 Latin Capital Letter R
    {0x46,0x49,0x49,0x49,0x31},     //   ( 51)  S - 0x0053 Latin Capital Letter S
    {0x01,0x01,0x7F,0x01,0x01},     //   ( 52)  T - 0x0054 Latin Capital Letter T
    {0x3F,0x40,0x40,0x40,0x3F},     //   ( 53)  U - 0x0055 Latin Capital Letter U
    {0x1F,0x20,0x40,0x20,0x1F},     //   ( 54)  V - 0x0056 Latin Capital Letter V
    {0x3F,0x40,0x38,0x40,0x3F},     //   ( 55)  W - 0x0057 Latin Capital Letter W
    {0x63,0x14,0x08,0x14,0x63},     //   ( 56)  X - 0x0058 Latin Capital Letter X
    {0x07,0x08,0x70,0x08,0x07},     //   ( 57)  Y - 0x0059 Latin Capital Letter Y
    {0x61,0x51,0x49,0x45,0x43},     //   ( 58)  Z - 0x005A Latin Capital Letter Z
    {0x00,0x7F,0x41,0x41,0x00},     //   ( 59)  [ - 0x005B Left Square Bracket
    {0x02,0x04,0x08,0x10,0x20},     //   ( 60)  \ - 0x005C Reverse Solidus
    {0x00,0x41,0x41,0x7F,0x00},     //   ( 61)  ] - 0x005D Right Square Bracket
    {0x04,0x02,0x01,0x02,0x04},     //   ( 62)  ^ - 0x005E Circumflex Accent
    {0x40,0x40,0x40,0x40,0x40},     //   ( 63)  _ - 0x005F Low Line
    {0x01,0x02,0x04,0x00,0x00},     //   ( 64)  ` - 0x0060 Grave Accent
    {0x20,0x54,0x54,0x54,0x78},     //   ( 65)  a - 0x0061 Latin Small Letter A
    {0x7F,0x48,0x44,0x44,0x38},     //   ( 66)  b - 0x0062 Latin Small Letter B
    {0x38,0x44,0x44,0x44,0x20},     //   ( 67)  c - 0x0063 Latin Small Letter C
    {0x38,0x44,0x44,0x48,0x7F},     //   ( 68)  d - 0x0064 Latin Small Letter D
    {0x38,0x54,0x54,0x54,0x18},     //   ( 69)  e - 0x0065 Latin Small Letter E
    {0x08,0x7E,0x09,0x01,0x02},     //   ( 70)  f - 0x0066 Latin Small Letter F
    {0x06,0x49,0x49,0x49,0x3F},     //   ( 71)  g - 0x0067 Latin Small Letter G
    {0x7F,0x08,0x04,0x04,0x78},     //   ( 72)  h - 0x0068 Latin Small Letter H
    {0x00,0x44,0x7D,0x40,0x00},     //   ( 73)  i - 0x0069 Latin Small Letter I
    {0x20,0x40,0x44,0x3D,0x00},     //   ( 74)  j - 0x006A Latin Small Letter J
    {0x7F,0x10,0x28,0x44,0x00},     //   ( 75)  k - 0x006B Latin Small Letter K
    {0x00,0x41,0x7F,0x40,0x00},     //   ( 76)  l - 0x006C Latin Small Letter L
    {0x7C,0x04,0x18,0x04,0x7C},     //   ( 77)  m - 0x006D Latin Small Letter M
    {0x7C,0x08,0x04,0x04,0x78},     //   ( 78)  n - 0x006E Latin Small Letter N
    {0x38,0x44,0x44,0x44,0x38},     //   ( 79)  o - 0x006F Latin Small Letter O
    {0x7C,0x14,0x14,0x14,0x08},     //   ( 80)  p - 0x0070 Latin Small Letter P
    {0x08,0x14,0x14,0x18,0x7C},     //   ( 81)  q - 0x0071 Latin Small Letter Q
    {0x7C,0x08,0x04,0x04,0x08},     //   ( 82)  r - 0x0072 Latin Small Letter R
    {0x48,0x54,0x54,0x54,0x20},     //   ( 83)  s - 0x0073 Latin Small Letter S
    {0x04,0x3F,0x44,0x40,0x20},     //   ( 84)  t - 0x0074 Latin Small Letter T
    {0x3C,0x40,0x40,0x20,0x7C},     //   ( 85)  u - 0x0075 Latin Small Letter U
    {0x1C,0x20,0x40,0x20,0x1C},     //   ( 86)  v - 0x0076 Latin Small Letter V
    {0x3C,0x40,0x30,0x40,0x3C},     //   ( 87)  w - 0x0077 Latin Small Letter W
    {0x44,0x28,0x10,0x28,0x44},     //   ( 88)  x - 0x0078 Latin Small Letter X
    {0x0C,0x50,0x50,0x50,0x3C},     //   ( 89)  y - 0x0079 Latin Small Letter Y
    {0x44,0x64,0x54,0x4C,0x44},     //   ( 90)  z - 0x007A Latin Small Letter Z
    {0x00,0x08,0x36,0x41,0x00},     //   ( 91)  { - 0x007B Left Curly Bracket
    {0x00,0x00,0x7F,0x00,0x00},     //   ( 92)  | - 0x007C Vertical Line
    {0x00,0x41,0x36,0x08,0x00},     //   ( 93)  } - 0x007D Right Curly Bracket
    {0x02,0x01,0x02,0x04,0x02},     //   ( 94)  ~ - 0x007E Tilde
    {0x3E,0x55,0x55,0x41,0x22},     //   ( 95)  C - 0x0080 <Control>
    {0x00,0x00,0x00,0x00,0x00},     //   ( 96)    - 0x00A0 No-Break Space
    {0x00,0x00,0x79,0x00,0x00},     //   ( 97)  ! - 0x00A1 Inverted Exclamation Mark
    {0x18,0x24,0x74,0x2E,0x24},     //   ( 98)  c - 0x00A2 Cent Sign
    {0x48,0x7E,0x49,0x42,0x40},     //   ( 99)  L - 0x00A3 Pound Sign
    {0x5D,0x22,0x22,0x22,0x5D},     //   (100)  o - 0x00A4 Currency Sign
    {0x15,0x16,0x7C,0x16,0x15},     //   (101)  Y - 0x00A5 Yen Sign
    {0x00,0x00,0x77,0x00,0x00},     //   (102)  | - 0x00A6 Broken Bar
    {0x0A,0x55,0x55,0x55,0x28},     //   (103)    - 0x00A7 Section Sign
    {0x00,0x01,0x00,0x01,0x00},     //   (104)  " - 0x00A8 Diaeresis
    {0x00,0x0A,0x0D,0x0A,0x04},     //   (105)    - 0x00AA Feminine Ordinal Indicator
    {0x08,0x14,0x2A,0x14,0x22},     //   (106) << - 0x00AB Left-Pointing Double Angle Quotation Mark
    {0x04,0x04,0x04,0x04,0x1C},     //   (107)    - 0x00AC Not Sign
    {0x00,0x08,0x08,0x08,0x00},     //   (108)  - - 0x00AD Soft Hyphen
    {0x01,0x01,0x01,0x01,0x01},     //   (109)    - 0x00AF Macron
    {0x00,0x02,0x05,0x02,0x00},     //   (110)    - 0x00B0 Degree Sign
    {0x44,0x44,0x5F,0x44,0x44},     //   (111) +- - 0x00B1 Plus-Minus Sign
    {0x00,0x00,0x04,0x02,0x01},     //   (112)  ` - 0x00B4 Acute Accent
    {0x7E,0x20,0x20,0x10,0x3E},     //   (113)  u - 0x00B5 Micro Sign
    {0x06,0x0F,0x7F,0x00,0x7F},     //   (114)    - 0x00B6 Pilcrow Sign
    {0x00,0x18,0x18,0x00,0x00},     //   (115)  . - 0x00B7 Middle Dot
    {0x00,0x40,0x50,0x20,0x00},     //   (116)    - 0x00B8 Cedilla
    {0x00,0x0A,0x0D,0x0A,0x00},     //   (117)    - 0x00BA Masculine Ordinal Indicator
    {0x22,0x14,0x2A,0x14,0x08},     //   (118) >> - 0x00BB Right-Pointing Double Angle Quotation Mark
    {0x17,0x08,0x34,0x2A,0x7D},     //   (119) /4 - 0x00BC Vulgar Fraction One Quarter
    {0x17,0x08,0x04,0x6A,0x59},     //   (120) /2 - 0x00BD Vulgar Fraction One Half
    {0x30,0x48,0x45,0x40,0x20},     //   (121)  ? - 0x00BF Inverted Question Mark
    {0x70,0x29,0x26,0x28,0x70},     //   (122) `A - 0x00C0 Latin Capital Letter A with Grave
    {0x70,0x28,0x26,0x29,0x70},     //   (123) 'A - 0x00C1 Latin Capital Letter A with Acute
    {0x70,0x2A,0x25,0x2A,0x70},     //   (124) ^A - 0x00C2 Latin Capital Letter A with Circumflex
    {0x72,0x29,0x26,0x29,0x70},     //   (125) ~A - 0x00C3 Latin Capital Letter A with Tilde
    {0x70,0x29,0x24,0x29,0x70},     //   (126) "A - 0x00C4 Latin Capital Letter A with Diaeresis
    {0x70,0x2A,0x2D,0x2A,0x70},     //   (127)  A - 0x00C5 Latin Capital Letter A with Ring Above
    {0x7E,0x11,0x7F,0x49,0x49},     //   (128) AE - 0x00C6 Latin Capital Letter Ae
    {0x0E,0x51,0x51,0x71,0x11},     //   (129)  C - 0x00C7 Latin Capital Letter C with Cedilla
    {0x7C,0x55,0x56,0x54,0x44},     //   (130) `E - 0x00C8 Latin Capital Letter E with Grave
    {0x7C,0x55,0x56,0x54,0x44},     //   (131) 'E - 0x00C9 Latin Capital Letter E with Acute
    {0x7C,0x56,0x55,0x56,0x44},     //   (132) ^E - 0x00CA Latin Capital Letter E with Circumflex
    {0x7C,0x55,0x54,0x55,0x44},     //   (133) "E - 0x00CB Latin Capital Letter E with Diaeresis
    {0x00,0x45,0x7E,0x44,0x00},     //   (134) `I - 0x00CC Latin Capital Letter I with Grave
    {0x00,0x44,0x7E,0x45,0x00},     //   (135) 'I - 0x00CD Latin Capital Letter I with Acute
    {0x00,0x46,0x7D,0x46,0x00},     //   (136) ^I - 0x00CE Latin Capital Letter I with Circumflex
    {0x00,0x45,0x7C,0x45,0x00},     //   (137) "I - 0x00CF Latin Capital Letter I with Diaeresis
    {0x7F,0x49,0x49,0x41,0x3E},     //   (138)  D - 0x00D0 Latin Capital Letter Eth
    {0x7C,0x0A,0x11,0x22,0x7D},     //   (139) ~N - 0x00D1 Latin Capital Letter N with Tilde
    {0x38,0x45,0x46,0x44,0x38},     //   (140) `O - 0x00D2 Latin Capital Letter O with Grave
    {0x38,0x44,0x46,0x45,0x38},     //   (141) 'O - 0x00D3 Latin Capital Letter O with Acute
    {0x38,0x46,0x45,0x46,0x38},     //   (142) ^O - 0x00D4 Latin Capital Letter O with Circumflex
    {0x38,0x46,0x45,0x46,0x39},     //   (143) ~O - 0x00D5 Latin Capital Letter O with Tilde
    {0x38,0x45,0x44,0x45,0x38},     //   (144) "O - 0x00D6 Latin Capital Letter O with Diaeresis
    {0x22,0x14,0x08,0x14,0x22},     //   (145)  x - 0x00D7 Multiplcation Sign
    {0x2E,0x51,0x49,0x45,0x3A},     //   (146)  O - 0x00D8 Latin Capital Letter O with Stroke
    {0x3C,0x41,0x42,0x40,0x3C},     //   (147) `U - 0x00D9 Latin Capital Letter U with Grave
    {0x3C,0x40,0x42,0x41,0x3C},     //   (148) 'U - 0x00DA Latin Capital Letter U with Acute
    {0x3C,0x42,0x41,0x42,0x3C},     //   (149) ^U - 0x00DB Latin Capital Letter U with Circumflex
    {0x3C,0x41,0x40,0x41,0x3C},     //   (150) "U - 0x00DC Latin Capital Letter U with Diaeresis
    {0x0C,0x10,0x62,0x11,0x0C},     //   (151) `Y - 0x00DD Latin Capital Letter Y with Acute
    {0x7F,0x12,0x12,0x12,0x0C},     //   (152)  P - 0x00DE Latin Capital Letter Thom
    {0x40,0x3E,0x01,0x49,0x36},     //   (153)  B - 0x00DF Latin Capital Letter Sharp S
    {0x20,0x55,0x56,0x54,0x78},     //   (154) `a - 0x00E0 Latin Small Letter A with Grave
    {0x20,0x54,0x56,0x55,0x78},     //   (155) 'a - 0x00E1 Latin Small Letter A with Acute
    {0x20,0x56,0x55,0x56,0x78},     //   (156) ^a - 0x00E2 Latin Small Letter A with Circumflex
    {0x20,0x55,0x56,0x55,0x78},     //   (157) ~a - 0x00E3 Latin Small Letter A with Tilde
    {0x20,0x55,0x54,0x55,0x78},     //   (158) "a - 0x00E4 Latin Small Letter A with Diaeresis
    {0x20,0x56,0x57,0x56,0x78},     //   (159)  a - 0x00E5 Latin Small Letter A with Ring Above
    {0x24,0x54,0x78,0x54,0x58},     //   (160) ae - 0x00E6 Latin Small Letter Ae
    {0x0C,0x52,0x52,0x72,0x13},     //   (161)  c - 0x00E7 Latin Small Letter c with Cedilla
    {0x38,0x55,0x56,0x54,0x18},     //   (162) `e - 0x00E8 Latin Small Letter E with Grave
    {0x38,0x54,0x56,0x55,0x18},     //   (163) 'e - 0x00E9 Latin Small Letter E with Acute
    {0x38,0x56,0x55,0x56,0x18},     //   (164) ^e - 0x00EA Latin Small Letter E with Circumflex
    {0x38,0x55,0x54,0x55,0x18},     //   (165) "e - 0x00EB Latin Small Letter E with Diaeresis
    {0x00,0x49,0x7A,0x40,0x00},     //   (166) `i - 0x00EC Latin Small Letter I with Grave
    {0x00,0x48,0x7A,0x41,0x00},     //   (167) 'i - 0x00ED Latin Small Letter I with Acute
    {0x00,0x4A,0x79,0x42,0x00},     //   (168) ^i - 0x00EE Latin Small Letter I with Circumflex
    {0x00,0x4A,0x78,0x42,0x00},     //   (169) "i - 0x00EF Latin Small Letter I with Diaeresis
    {0x31,0x4A,0x4E,0x4A,0x30},     //   (170)    - 0x00F0 Latin Small Letter Eth
    {0x7A,0x11,0x0A,0x09,0x70},     //   (171) ~n - 0x00F1 Latin Small Letter N with Tilde
    {0x30,0x49,0x4A,0x48,0x30},     //   (172) `o - 0x00F2 Latin Small Letter O with Grave
    {0x30,0x48,0x4A,0x49,0x30},     //   (173) 'o - 0x00F3 Latin Small Letter O with Acute
    {0x30,0x4A,0x49,0x4A,0x30},     //   (174) ^o - 0x00F4 Latin Small Letter O with Circumflex
    {0x30,0x4A,0x49,0x4A,0x31},     //   (175) ~o - 0x00F5 Latin Small Letter O with Tilde
    {0x30,0x4A,0x48,0x4A,0x30},     //   (176) "o - 0x00F6 Latin Small Letter O with Diaeresis
    {0x08,0x08,0x2A,0x08,0x08},     //   (177)  + - 0x00F7 Division Sign
    {0x38,0x64,0x54,0x4C,0x38},     //   (178)  o - 0x00F8 Latin Small Letter O with Stroke
    {0x38,0x41,0x42,0x20,0x78},     //   (179) `u - 0x00F9 Latin Small Letter U with Grave
    {0x38,0x40,0x42,0x21,0x78},     //   (180) 'u - 0x00FA Latin Small Letter U with Acute
    {0x38,0x42,0x41,0x22,0x78},     //   (181) ^u - 0x00FB Latin Small Letter U with Circumflex
    {0x38,0x42,0x40,0x22,0x78},     //   (182) "u - 0x00FC Latin Small Letter U with Diaeresis
    {0x0C,0x50,0x52,0x51,0x3C},     //   (183) 'y - 0x00FD Latin Small Letter Y with Acute
    {0x7E,0x14,0x14,0x14,0x08},     //   (184)  p - 0x00FE Latin Small Letter Thom
    {0x0C,0x51,0x50,0x51,0x3C},     //   (185) "y - 0x00FF Latin Small Letter Y with Diaeresis
    {0x1E,0x09,0x09,0x29,0x5E},     //   (186)  A - 0x0104 Latin Capital Letter A with Ogonek
    {0x08,0x15,0x15,0x35,0x4E},     //   (187)  a - 0x0105 Latin Small Letter A with Ogonek
    {0x38,0x44,0x46,0x45,0x20},     //   (188) 'C - 0x0106 Latin Capital Letter C with Acute
    {0x30,0x48,0x4A,0x49,0x20},     //   (189) 'c - 0x0107 Latin Small Letter C with Acute
    {0x38,0x45,0x46,0x45,0x20},     //   (190)  C - 0x010C Latin Capital Letter C with Caron
    {0x30,0x49,0x4A,0x49,0x20},     //   (191)  c - 0x010D Latin Small Letter C with Caron
    {0x7C,0x45,0x46,0x45,0x38},     //   (192)  D - 0x010E Latin Capital Letter D with Caron
    {0x20,0x50,0x50,0x7C,0x03},     //   (193) d' - 0x010F Latin Small Letter D with Caron
    {0x1F,0x15,0x15,0x35,0x51},     //   (194)  E - 0x0118 Latin Capital Letter E with Ogonek
    {0x0E,0x15,0x15,0x35,0x46},     //   (195)  e - 0x0119 Latin Small Letter E with Ogonek
    {0x7C,0x55,0x56,0x55,0x44},     //   (196)  E - 0x011A Latin Capital Letter E with Caron
    {0x38,0x55,0x56,0x55,0x18},     //   (197)  e - 0x011B Latin Small Letter E with Caron
    {0x00,0x44,0x7C,0x40,0x00},     //   (198)  i - 0x0131 Latin Small Letter Dotless I
    {0x7F,0x48,0x44,0x40,0x40},     //   (199)  L - 0x0141 Latin Capital Letter L with Stroke
    {0x00,0x49,0x7F,0x44,0x00},     //   (200)  l - 0x0142 Latin Small Letter L with Stroke
    {0x7C,0x08,0x12,0x21,0x7C},     //   (201) 'N - 0x0143 Latin Capital Letter N with Acute
    {0x78,0x10,0x0A,0x09,0x70},     //   (202) 'n - 0x0144 Latin Small Letter N with Acute
    {0x7C,0x09,0x12,0x21,0x7C},     //   (203)  N - 0x0147 Latin Capital Letter N with Caron
    {0x78,0x11,0x0A,0x09,0x70},     //   (204)  n - 0x0148 Latin Small Letter N with Caron
    {0x38,0x47,0x44,0x47,0x38},     //   (205) "O - 0x0150 Latin Capital Letter O with Double Acute
    {0x30,0x4B,0x48,0x4B,0x30},     //   (206) "o - 0x0151 Latin Small Letter O with Double Acute
    {0x3E,0x41,0x7F,0x49,0x49},     //   (207) OE - 0x0152 Latin Capital Ligature Oe
    {0x38,0x44,0x38,0x54,0x58},     //   (208) oe - 0x0153 Latin Small Ligature Oe
    {0x7C,0x15,0x16,0x35,0x48},     //   (209)  R - 0x0158 Latin Capital Letter R with Caron
    {0x78,0x11,0x0A,0x09,0x10},     //   (210)  r - 0x0159 Latin Small Letter R with Caron
    {0x48,0x54,0x56,0x55,0x20},     //   (211) 'S - 0x015A Latin Capital Letter S with Acute
    {0x20,0x48,0x56,0x55,0x20},     //   (212) 's - 0x015B Latin Small Letter S with Acute
    {0x48,0x55,0x56,0x55,0x20},     //   (213)  S - 0x0160 Latin Capital Letter S with Caron
    {0x20,0x49,0x56,0x55,0x20},     //   (214)  s - 0x0161 Latin Small Letter S with Caron
    {0x04,0x05,0x7E,0x05,0x04},     //   (215)  T - 0x0164 Latin Capital Letter T with Caron
    {0x08,0x3C,0x48,0x22,0x01},     //   (216) t' - 0x0165 Latin Small Letter T with Caron
    {0x3C,0x42,0x45,0x42,0x3C},     //   (217)  U - 0x016E Latin Capital Letter U with Ring Above
    {0x38,0x42,0x45,0x22,0x78},     //   (218)  u - 0x016F Latin Small Letter U with Ring Above
    {0x3C,0x43,0x40,0x43,0x3C},     //   (219) "U - 0x0170 Latin Capital Letter U with Double Acute
    {0x38,0x43,0x40,0x23,0x78},     //   (220) "u - 0x0171 Latin Small Letter U with Double Acute
    {0x0C,0x11,0x60,0x11,0x0C},     //   (221) "Y - 0x0178 Latin Capital Letter Y with Diaeresis
    {0x44,0x66,0x55,0x4C,0x44},     //   (222) 'Z - 0x0179 Latin Capital Letter Z with Acute
    {0x48,0x6A,0x59,0x48,0x00},     //   (223) 'z - 0x017A Latin Small Letter Z with Acute
    {0x44,0x64,0x55,0x4C,0x44},     //   (224)  Z - 0x017B Latin Capital Letter Z with Dot Above
    {0x48,0x68,0x5A,0x48,0x00},     //   (225)  z - 0x017C Latin Small Letter Z with Dot Above
    {0x44,0x65,0x56,0x4D,0x44},     //   (226)  Z - 0x017D Latin Capital Letter Z with Caron
    {0x48,0x69,0x5A,0x49,0x00},     //   (227)  z - 0x017E Latin Small Letter Z with Caron
    {0x00,0x02,0x01,0x02,0x00},     //   (228)  ^ - 0x02C6 Modifier Letter Circumflex Accent
    {0x00,0x01,0x02,0x01,0x00},     //   (229)    - 0x02C7 Caron
    {0x00,0x01,0x01,0x01,0x00},     //   (230)    - 0x02C9 Modifier Letter Macron
    {0x01,0x02,0x02,0x01,0x00},     //   (231)    - 0x02D8 Breve
    {0x00,0x00,0x01,0x00,0x00},     //   (232)    - 0x02D9 Dot Above
    {0x00,0x02,0x05,0x02,0x00},     //   (233)    - 0x02DA Ring Above
    {0x02,0x01,0x02,0x01,0x00},     //   (234)  ~ - 0x02DC Small Tilde
    {0x7F,0x05,0x15,0x3A,0x50},     //   (235) Pt - 0x20A7 Peseta Sign
    {0x3E,0x55,0x55,0x41,0x22},     //   (236)  C - 0x20AC Euro Sign
    {0x18,0x14,0x08,0x14,0x0C},     //   (237)    - 0x221E Infinity
    {0x44,0x4A,0x4A,0x51,0x51},     //   (238)  < - 0x2264 Less-Than or Equal to
    {0x51,0x51,0x4A,0x4A,0x44},     //   (239)  > - 0x2265 Greater-Than or Equal to
    {0x74,0x42,0x41,0x42,0x74},     //   (240)    - 0x2302 House
};


const unsigned char Ascii_2[108][5]={        // Refer to "Times New Roman" Font Database...
                        //   Greek & Japanese Letters
    {0x00,0x00,0x00,0x00,0x00},     //
    {0x7E,0x11,0x11,0x11,0x7E},     //   (  1)  A - 0x0391 Greek Capital Letter Alpha
    {0x7F,0x49,0x49,0x49,0x36},     //   (  2)  B - 0x0392 Greek Capital Letter Beta
    {0x7F,0x02,0x01,0x01,0x03},     //   (  3)    - 0x0393 Greek Capital Letter Gamma
    {0x70,0x4E,0x41,0x4E,0x70},     //   (  4)    - 0x0394 Greek Capital Letter Delta
    {0x7F,0x49,0x49,0x49,0x41},     //   (  5)  E - 0x0395 Greek Capital Letter Epsilon
    {0x61,0x51,0x49,0x45,0x43},     //   (  6)  Z - 0x0396 Greek Capital Letter Zeta
    {0x7F,0x08,0x08,0x08,0x7F},     //   (  7)  H - 0x0397 Greek Capital Letter Eta
    {0x3E,0x49,0x49,0x49,0x3E},     //   (  8)    - 0x0398 Greek Capital Letter Theta
    {0x00,0x41,0x7F,0x41,0x00},     //   (  9)  I - 0x0399 Greek Capital Letter Iota
    {0x7F,0x08,0x14,0x22,0x41},     //   ( 10)  K - 0x039A Greek Capital Letter Kappa
    {0x70,0x0E,0x01,0x0E,0x70},     //   ( 11)    - 0x039B Greek Capital Letter Lamda
    {0x7F,0x02,0x0C,0x02,0x7F},     //   ( 12)  M - 0x039C Greek Capital Letter Mu
    {0x7F,0x04,0x08,0x10,0x7F},     //   ( 13)  N - 0x039D Greek Capital Letter Nu
    {0x63,0x5D,0x49,0x5D,0x63},     //   ( 14)    - 0x039E Greek Capital Letter Xi
    {0x3E,0x41,0x41,0x41,0x3E},     //   ( 15)  O - 0x039F Greek Capital Letter Omicron
    {0x41,0x3F,0x01,0x3F,0x41},     //   ( 16)    - 0x03A0 Greek Capital Letter Pi
    {0x7F,0x09,0x09,0x09,0x06},     //   ( 17)  P - 0x03A1 Greek Capital Letter Rho
    {0x63,0x55,0x49,0x41,0x41},     //   ( 18)    - 0x03A3 Greek Capital Letter Sigma
    {0x01,0x01,0x7F,0x01,0x01},     //   ( 19)  T - 0x03A4 Greek Capital Letter Tau
    {0x03,0x01,0x7E,0x01,0x03},     //   ( 20)    - 0x03A5 Greek Capital Letter Upsilon
    {0x08,0x55,0x7F,0x55,0x08},     //   ( 21)    - 0x03A6 Greek Capital Letter Phi
    {0x63,0x14,0x08,0x14,0x63},     //   ( 22)  X - 0x03A7 Greek Capital Letter Chi
    {0x07,0x48,0x7F,0x48,0x07},     //   ( 23)    - 0x03A8 Greek Capital Letter Psi
    {0x5E,0x61,0x01,0x61,0x5E},     //   ( 24)    - 0x03A9 Greek Capital Letter Omega
    {0x38,0x44,0x48,0x30,0x4C},     //   ( 25)  a - 0x03B1 Greek Small Letter Alpha
    {0x7C,0x2A,0x2A,0x2A,0x14},     //   ( 26)  B - 0x03B2 Greek Small Letter Beta
    {0x44,0x38,0x04,0x04,0x08},     //   ( 27)  r - 0x03B3 Greek Small Letter Gamma
    {0x30,0x4B,0x4D,0x59,0x30},     //   ( 28)    - 0x03B4 Greek Small Letter Delta
    {0x28,0x54,0x54,0x44,0x20},     //   ( 29)    - 0x03B5 Greek Small Letter Epsilon
    {0x00,0x18,0x55,0x52,0x22},     //   ( 30)    - 0x03B6 Greek Small Letter Zeta
    {0x3E,0x04,0x02,0x02,0x7C},     //   ( 31)  n - 0x03B7 Greek Small Letter Eta
    {0x3C,0x4A,0x4A,0x4A,0x3C},     //   ( 32)    - 0x03B8 Greek Small Letter Theta
    {0x00,0x3C,0x40,0x20,0x00},     //   ( 33)  i - 0x03B9 Greek Small Letter Iota
    {0x7C,0x10,0x28,0x44,0x40},     //   ( 34)  k - 0x03BA Greek Small Letter Kappa
    {0x41,0x32,0x0C,0x30,0x40},     //   ( 35)    - 0x03BB Greek Small Letter Lamda
    {0x7E,0x20,0x20,0x10,0x3E},     //   ( 36)  u - 0x03BC Greek Small Letter Mu
    {0x1C,0x20,0x40,0x20,0x1C},     //   ( 37)  v - 0x03BD Greek Small Letter Nu
    {0x14,0x2B,0x2A,0x2A,0x60},     //   ( 38)    - 0x03BE Greek Small Letter Xi
    {0x38,0x44,0x44,0x44,0x38},     //   ( 39)  o - 0x03BF Greek Small Letter Omicron
    {0x44,0x3C,0x04,0x7C,0x44},     //   ( 40)    - 0x03C0 Greek Small Letter Pi
    {0x70,0x28,0x24,0x24,0x18},     //   ( 41)  p - 0x03C1 Greek Small Letter Rho
    {0x0C,0x12,0x12,0x52,0x60},     //   ( 42)    - 0x03C2 Greek Small Letter Final Sigma
    {0x38,0x44,0x4C,0x54,0x24},     //   ( 43)    - 0x03C3 Greek Small Letter Sigma
    {0x04,0x3C,0x44,0x20,0x00},     //   ( 44)  t - 0x03C4 Greek Small Letter Tau
    {0x3C,0x40,0x40,0x20,0x1C},     //   ( 45)  v - 0x03C5 Greek Small Letter Upsilon
    {0x18,0x24,0x7E,0x24,0x18},     //   ( 46)    - 0x03C6 Greek Small Letter Phi
    {0x44,0x28,0x10,0x28,0x44},     //   ( 47)  x - 0x03C7 Greek Small Letter Chi
    {0x0C,0x10,0x7E,0x10,0x0C},     //   ( 48)    - 0x03C8 Greek Small Letter Psi
    {0x38,0x44,0x30,0x44,0x38},     //   ( 49)  w - 0x03C9 Greek Small Letter Omega
    {0x0A,0x0A,0x4A,0x2A,0x1E},     //   ( 50)    - 0xFF66 Katakana Letter Wo
    {0x04,0x44,0x34,0x14,0x0C},     //   ( 51)    - 0xFF67 Katakana Letter Small A
    {0x20,0x10,0x78,0x04,0x00},     //   ( 52)    - 0xFF68 Katakana Letter Small I
    {0x18,0x08,0x4C,0x48,0x38},     //   ( 53)    - 0xFF69 Katakana Letter Small U
    {0x48,0x48,0x78,0x48,0x48},     //   ( 54)    - 0xFF6A Katakana Letter Small E
    {0x48,0x28,0x18,0x7C,0x08},     //   ( 55)    - 0xFF6B Katakana Letter Small O
    {0x08,0x7C,0x08,0x28,0x18},     //   ( 56)    - 0xFF6C Katakana Letter Small Ya
    {0x40,0x48,0x48,0x78,0x40},     //   ( 57)    - 0xFF6D Katakana Letter Small Yu
    {0x54,0x54,0x54,0x7C,0x00},     //   ( 58)    - 0xFF6E Katakana Letter Small Yo
    {0x18,0x00,0x58,0x40,0x38},     //   ( 59)    - 0xFF6F Katakana Letter Small Tu
    {0x08,0x08,0x08,0x08,0x08},     //   ( 60)    - 0xFF70 Katakana-Hiragana Prolonged Sound Mark
    {0x01,0x41,0x3D,0x09,0x07},     //   ( 61)    - 0xFF71 Katakana Letter A
    {0x10,0x08,0x7C,0x02,0x01},     //   ( 62)    - 0xFF72 Katakana Letter I
    {0x0E,0x02,0x43,0x22,0x1E},     //   ( 63)    - 0xFF73 Katakana Letter U
    {0x42,0x42,0x7E,0x42,0x42},     //   ( 64)    - 0xFF74 Katakana Letter E
    {0x22,0x12,0x0A,0x7F,0x02},     //   ( 65)    - 0xFF75 Katakana Letter O
    {0x42,0x3F,0x02,0x42,0x3E},     //   ( 66)    - 0xFF76 Katakana Letter Ka
    {0x0A,0x0A,0x7F,0x0A,0x0A},     //   ( 67)    - 0xFF77 Katakana Letter Ki
    {0x08,0x46,0x42,0x22,0x1E},     //   ( 68)    - 0xFF78 Katakana Letter Ku
    {0x04,0x03,0x42,0x3E,0x02},     //   ( 69)    - 0xFF79 Katakana Letter Ke
    {0x42,0x42,0x42,0x42,0x7E},     //   ( 70)    - 0xFF7A Katakana Letter Ko
    {0x02,0x4F,0x22,0x1F,0x02},     //   ( 71)    - 0xFF7B Katakana Letter Sa
    {0x4A,0x4A,0x40,0x20,0x1C},     //   ( 72)    - 0xFF7C Katakana Letter Shi
    {0x42,0x22,0x12,0x2A,0x46},     //   ( 73)    - 0xFF7D Katakana Letter Su
    {0x02,0x3F,0x42,0x4A,0x46},     //   ( 74)    - 0xFF7E Katakana Letter Se
    {0x06,0x48,0x40,0x20,0x1E},     //   ( 75)    - 0xFF7F Katakana Letter So
    {0x08,0x46,0x4A,0x32,0x1E},     //   ( 76)    - 0xFF80 Katakana Letter Ta
    {0x0A,0x4A,0x3E,0x09,0x08},     //   ( 77)    - 0xFF81 Katakana Letter Chi
    {0x0E,0x00,0x4E,0x20,0x1E},     //   ( 78)    - 0xFF82 Katakana Letter Tsu
    {0x04,0x45,0x3D,0x05,0x04},     //   ( 79)    - 0xFF83 Katakana Letter Te
    {0x00,0x7F,0x08,0x10,0x00},     //   ( 80)    - 0xFF84 Katakana Letter To
    {0x44,0x24,0x1F,0x04,0x04},     //   ( 81)    - 0xFF85 Katakana Letter Na
    {0x40,0x42,0x42,0x42,0x40},     //   ( 82)    - 0xFF86 Katakana Letter Ni
    {0x42,0x2A,0x12,0x2A,0x06},     //   ( 83)    - 0xFF87 Katakana Letter Nu
    {0x22,0x12,0x7B,0x16,0x22},     //   ( 84)    - 0xFF88 Katakana Letter Ne
    {0x00,0x40,0x20,0x1F,0x00},     //   ( 85)    - 0xFF89 Katakana Letter No
    {0x78,0x00,0x02,0x04,0x78},     //   ( 86)    - 0xFF8A Katakana Letter Ha
    {0x3F,0x44,0x44,0x44,0x44},     //   ( 87)    - 0xFF8B Katakana Letter Hi
    {0x02,0x42,0x42,0x22,0x1E},     //   ( 88)    - 0xFF8C Katakana Letter Fu
    {0x04,0x02,0x04,0x08,0x30},     //   ( 89)    - 0xFF8D Katakana Letter He
    {0x32,0x02,0x7F,0x02,0x32},     //   ( 90)    - 0xFF8E Katakana Letter Ho
    {0x02,0x12,0x22,0x52,0x0E},     //   ( 91)    - 0xFF8F Katakana Letter Ma
    {0x00,0x2A,0x2A,0x2A,0x40},     //   ( 92)    - 0xFF90 Katakana Letter Mi
    {0x38,0x24,0x22,0x20,0x70},     //   ( 93)    - 0xFF91 Katakana Letter Mu
    {0x40,0x28,0x10,0x28,0x06},     //   ( 94)    - 0xFF92 Katakana Letter Me
    {0x0A,0x3E,0x4A,0x4A,0x4A},     //   ( 95)    - 0xFF93 Katakana Letter Mo
    {0x04,0x7F,0x04,0x14,0x0C},     //   ( 96)    - 0xFF94 Katakana Letter Ya
    {0x40,0x42,0x42,0x7E,0x40},     //   ( 97)    - 0xFF95 Katakana Letter Yu
    {0x4A,0x4A,0x4A,0x4A,0x7E},     //   ( 98)    - 0xFF96 Katakana Letter Yo
    {0x04,0x05,0x45,0x25,0x1C},     //   ( 99)    - 0xFF97 Katakana Letter Ra
    {0x0F,0x40,0x20,0x1F,0x00},     //   (100)    - 0xFF98 Katakana Letter Ri
    {0x7C,0x00,0x7E,0x40,0x30},     //   (101)    - 0xFF99 Katakana Letter Ru
    {0x7E,0x40,0x20,0x10,0x08},     //   (102)    - 0xFF9A Katakana Letter Re
    {0x7E,0x42,0x42,0x42,0x7E},     //   (103)    - 0xFF9B Katakana Letter Ro
    {0x0E,0x02,0x42,0x22,0x1E},     //   (104)    - 0xFF9C Katakana Letter Wa
    {0x42,0x42,0x40,0x20,0x18},     //   (105)    - 0xFF9D Katakana Letter N
    {0x02,0x04,0x01,0x02,0x00},     //   (106)    - 0xFF9E Katakana Voiced Sound Mark
    {0x07,0x05,0x07,0x00,0x00},     //   (107)    - 0xFF9F Katakana Semi-Voiced Sound Mark
};


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Graphic Acceleration (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//    e: Line Width
//    f: Line Color - RRRRRGGG
//    g: Line Color - GGGBBBBB
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g)
{
unsigned char i,j;

    Set_Display_Offset(a,c);
    Set_Column_Address(a,b);
    Set_Row_Address(c,(c+e-1));
    Set_Write_RAM();
    for(i=0;i<(b-a+1);i++)
    {
        for(j=0;j<e;j++)
        {
            Write_Data(f);      // Line Color - RRRRRGGG
            Write_Data(g);      // Line Color - GGGBBBBB
        }
    }

    Set_Display_Offset((b-e+1),c);
    Set_Column_Address((b-e+1),b);
    Set_Row_Address(c,d);
    Set_Write_RAM();
    for(i=0;i<(d-c+1);i++)
    {
        for(j=0;j<e;j++)
        {
            Write_Data(f);      // Line Color - RRRRRGGG
            Write_Data(g);      // Line Color - GGGBBBBB
        }
    }

    Set_Display_Offset(a,(d-e+1));
    Set_Column_Address(a,b);
    Set_Row_Address((d-e+1),d);
    Set_Write_RAM();
    for(i=0;i<(b-a+1);i++)
    {
        for(j=0;j<e;j++)
        {
            Write_Data(f);      // Line Color - RRRRRGGG
            Write_Data(g);      // Line Color - GGGBBBBB
        }
    }

    Set_Display_Offset(a,c);
    Set_Column_Address(a,(a+e-1));
    Set_Row_Address(c,d);
    Set_Write_RAM();
    for(i=0;i<(d-c+1);i++)
    {
        for(j=0;j<e;j++)
        {
            Write_Data(f);      // Line Color - RRRRRGGG
            Write_Data(g);      // Line Color - GGGBBBBB
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//
//    a: RRRRRGGG
//    b: GGGBBBBB
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM(unsigned char a, unsigned char b)
{
unsigned char i,j;

    Set_Display_Offset(0x00,0x00);
    Set_Column_Address(0x00,0x9F);
    Set_Row_Address(0x00,0x7F);
    Set_Write_RAM();

    for(i=0;i<128;i++)
    {
        for(j=0;j<160;j++)
        {
            Write_Data(a);
            Write_Data(b);
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//    e: RRRRRGGG
//    f: GGGBBBBB
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_Block(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
unsigned char i,j;

    Set_Display_Offset(a,c);
    Set_Column_Address(a,b);
    Set_Row_Address(c,d);
    Set_Write_RAM();

    for(i=0;i<(d-c+1);i++)
    {
        for(j=0;j<(b-a+1);j++)
        {
            Write_Data(e);
            Write_Data(f);
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Checkboard (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Checkerboard()
{
unsigned char i,j;

    Set_Display_Offset(0x00,0x00);
    Set_Column_Address(0x00,0x9F);
    Set_Row_Address(0x00,0x7F);
    Set_Write_RAM();

    for(i=0;i<64;i++)
    {
        for(j=0;j<80;j++)
        {
            Write_Data(0xFF);
            Write_Data(0xFF);
            Write_Data(0x00);
            Write_Data(0x00);
        }
        for(j=0;j<80;j++)
        {
            Write_Data(0x00);
            Write_Data(0x00);
            Write_Data(0xFF);
            Write_Data(0xFF);
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Color Bar (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Rainbow()
{
    // White => Column 1~20
        Fill_Block(0x00,0x13,0x00,Max_Row,0xFF,0xFF);

    // Yellow => Column 21~40
        Fill_Block(0x14,0x27,0x00,Max_Row,0xFF,0xE0);

    // Purple => Column 41~60
        Fill_Block(0x28,0x3B,0x00,Max_Row,0xF8,0x1F);

    // Cyan => Column 61~80
        Fill_Block(0x3C,0x4F,0x00,Max_Row,0x07,0xFF);

    // Red => Column 81~100
        Fill_Block(0x50,0x63,0x00,Max_Row,0xF8,0x00);

    // Green => Column 101~120
        Fill_Block(0x64,0x77,0x00,Max_Row,0x07,0xE0);

    // Blue => Column 121~140
        Fill_Block(0x78,0x8B,0x00,Max_Row,0x00,0x1F);

    // Black => Column 141~160
        Fill_Block(0x8C,Max_Column,0x00,Max_Row,0x00,0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Character (5x7)
//
//    a: Database
//    b: Ascii
//    c: Data Format (First Transmission)
//       D[15:8]=[RRRRR:GGG] (65,536 Colors Mode)
//    d: Data Format (Second Transmission)
//       D[7:0]=[GGG:BBBBB] (65,536 Colors Mode)
//    e: Start X Address
//    f: Start Y Address
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f)
{
unsigned char *Src_Pointer=NULL;
unsigned char i,Font,Pick;

    switch(a)
    {
        case 1:
            Src_Pointer=(unsigned char *)&Ascii_1[(b-1)][0];
            break;
        case 2:
            Src_Pointer=(unsigned char *)&Ascii_2[(b-1)][0];
            break;
    }

    Set_Pixel_Format(0x67|((Color_Depth<<4)&0x70));
    Src_Pointer-=4;
    for(i=0;i<=5;i++)
    {
        Pick=*Src_Pointer;
        Set_Display_Offset(e+i,f);
        Set_Column_Address(e+i,e+i);
        Set_Row_Address(f,f+7);
        Set_Write_RAM();

        Font=Pick&0x01;
        Font=Font|(Font<<1)|(Font<<2)|(Font<<3)|(Font<<4)|(Font<<5)|(Font<<6)|(Font<<7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x02)>>1;
        Font=Font|(Font<<1)|(Font<<2)|(Font<<3)|(Font<<4)|(Font<<5)|(Font<<6)|(Font<<7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x04)>>2;
        Font=Font|(Font<<1)|(Font<<2)|(Font<<3)|(Font<<4)|(Font<<5)|(Font<<6)|(Font<<7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x08)>>3;
        Font=Font|(Font<<1)|(Font<<2)|(Font<<3)|(Font<<4)|(Font<<5)|(Font<<6)|(Font<<7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x10)<<3;
        Font=Font|(Font>>1)|(Font>>2)|(Font>>3)|(Font>>4)|(Font>>5)|(Font>>6)|(Font>>7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x20)<<2;
        Font=Font|(Font>>1)|(Font>>2)|(Font>>3)|(Font>>4)|(Font>>5)|(Font>>6)|(Font>>7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=(Pick&0x40)<<1;
        Font=Font|(Font>>1)|(Font>>2)|(Font>>3)|(Font>>4)|(Font>>5)|(Font>>6)|(Font>>7);
        Write_Data(c&Font);
        Write_Data(d&Font);

        Font=Pick&0x80;
        Font=Font|(Font>>1)|(Font>>2)|(Font>>3)|(Font>>4)|(Font>>5)|(Font>>6)|(Font>>7);
        Write_Data(c&Font);
        Write_Data(d&Font);
        Src_Pointer++;
    }
    Set_Display_Offset(e+5,f);
    Set_Column_Address(e+5,e+5);
    Set_Row_Address(f,f+7);
    Set_Write_RAM();

    for(i=0;i<=8;i++)
    {
        Write_Data(0x00);
        Write_Data(0x00);
    }
    Set_Pixel_Format(0x66|((Color_Depth<<4)&0x70));
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String
//
//    a: Database
//    b: Data Format (First Transmission)
//       D[15:8]=[RRRRR:GGG] (65,536 Colors Mode)
//    c: Data Format (Second Transmission)
//       D[7:0]=[GGG:BBBBB] (65,536 Colors Mode)
//    d: Start X Address
//    e: Start Y Address
//    * Must write "0" in the end...
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c, unsigned char d, unsigned char e)
{
unsigned char *Src_Pointer;

    Src_Pointer=Data_Pointer;
    Show_Font57(1,96,b,c,d,e);      // No-Break Space
                        //   Must be written first before the string start...

    while(1)
    {
        Show_Font57(a,*Src_Pointer,b,c,d,e);
        Src_Pointer++;
        d+=6;
        if(*Src_Pointer == 0) break;
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_64k_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char *Src_Pointer;
unsigned char i,j;

    Src_Pointer=Data_Pointer;
    Set_Display_Offset(a,c);
    Set_Column_Address(a,b);
    Set_Row_Address(c,d);
    Set_Write_RAM();

    for(i=0;i<(d-c+1);i++)
    {
        for(j=0;j<(b-a+1);j++)
        {
            Write_Data(*Src_Pointer);
            Src_Pointer++;
            Write_Data(*Src_Pointer);
            Src_Pointer++;
        }
    }
}


void Show_256k_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char *Src_Pointer;
unsigned char i,j;

    Src_Pointer=Data_Pointer;
    Set_Display_Offset(a,c);
    Set_Column_Address(a,b);
    Set_Row_Address(c,d);
    Set_Write_RAM();

    for(i=0;i<(d-c+1);i++)
    {
        for(j=0;j<(b-a+1);j++)
        {
            Write_Data((*Src_Pointer)<<2);
            Src_Pointer++;
            Write_Data((*Src_Pointer)<<2);
            Src_Pointer++;
            Write_Data((*Src_Pointer)<<2);
            Src_Pointer++;
        }
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Scrolling - Upward
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Up_Scroll()
{
unsigned int i,j,counter;

    counter=1;
    for(i=0;i<Max_Row;i++)
    {
        Set_Start_Line(counter);
        for(j=0;j<100;j++)
        {
            SEPS525_DELAY_US(200);
        }
        counter++;
    }
    Set_Start_Line(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical Scrolling - Downward
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Down_Scroll()
{
unsigned int i,j,counter;

    counter=Max_Row-1;
    for(i=0;i<Max_Row;i++)
    {
        Set_Start_Line(counter);
        for(j=0;j<100;j++)
        {
            SEPS525_DELAY_US(200);
        }
        counter--;
    }
    Set_Start_Line(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  First Screen Active Range (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void First_Screen(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    Write_Command(0x2E);            // Set Column Address of Start Point (D1_DDRAM_FAC)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x2F);            // Set Row Address of Start Point (D1_DDRAM_FAR)
    Write_Data(c);              //   Default => 0x9F
    Write_Command(0x33);            // Set Column Address of Start (SCR1_FX1)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x34);            // Set Column Address of End (SCR1_FX2)
    Write_Data(b);              //   Default => 0x9F
    Write_Command(0x35);            // Set Row Address of Start (SCR1_FY1)
    Write_Data(c);              //   Default => 0x00
    Write_Command(0x36);            // Set Row Address of End (SCR1_FY2)
    Write_Data(d);              //   Default => 0x7F
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Second Screen Active Range (Partial or Full Screen)
//
//    a: Column Address of Start
//    b: Column Address of End
//    c: Row Address of Start
//    d: Row Address of End
//    e: Column Address of Start for the Image Box
//    f: Column Address of End for the Image Box
//    g: Row Address of Start for the Image Box
//    h: Row Address of End for the Image Box
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Second_Screen(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h)
{
    Write_Command(0x31);            // Set Column Address of Start Point (D2_DDRAM_FAC)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x32);            // Set Row Address of Start Point (D2_DDRAM_FAR)
    Write_Data(c);              //   Default => 0x9F
    Write_Command(0x37);            // Set Column Address of Start (SCR2_SX1)
    Write_Data(a);              //   Default => 0x00
    Write_Command(0x38);            // Set Column Address of End (SCR2_SX2)
    Write_Data(b);              //   Default => 0x9F
    Write_Command(0x39);            // Set Row Address of Start (SCR2_SY1)
    Write_Data(c);              //   Default => 0x00
    Write_Command(0x3A);            // Set Row Address of End (SCR2_SY2)
    Write_Data(d);              //   Default => 0x7F
    Write_Command(0x47);            // Set Column Address of Start for the Image Box (SS_SCR2_SX1)
    Write_Data(e);              //   Default => 0x00
    Write_Command(0x48);            // Set Column Address of End for the Image Box (SS_SCR2_SX2)
    Write_Data(f);              //   Default => 0x00
    Write_Command(0x49);            // Set Row Address of Start for the Image Box (SS_SCR2_SY1)
    Write_Data(g);              //   Default => 0x00
    Write_Command(0x4A);            // Set Row Address of End for the Image Box (SS_SCR2_SY2)
    Write_Data(h);              //   Default => 0x00
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Screen Saver Control (Partial or Full Screen)
//
//    a: Screen Saver Mode On/Off
//       "0x00" (Deactivate Screen Saver)
//       "0x01" (Activate Screen Saver)
//    b: Screen Saver On/Off Control
//       "0x00" (Disable First & Second Screen Saver Both)
//       "0x01" (Enable First Screen Saver Only)
//       "0x02" (Enable Second Screen Saver Only)
//       "0x03" (Enable First & Second Screen Saver Both)
//    c: Screen Saver Mode Select for the First Screen
//       "0x01" (Left Panning)
//       "0x02" (Right Panning)
//    d: Set Numbers of Scroll per Step for the First Screen
//    e: Set Time Interval between Each Step in Terms of Frame Frequency for the First Screen
//    f: Screen Saver Mode Select for the Second Screen
//       "0x00" (Box Move)
//       "0x01" (Log On)
//       "0x03" (Wrap Around)
//    g: Set Numbers of Scroll per Step for the Second Screen
//    h: Scrolling Direction for the Second Screen
//       "0x00" (Up & Leftward)
//       "0x01" (Up & Rightward)
//       "0x02" (Down & Leftward)
//       "0x03" (Down & Rightward)
//    i: Set Time Interval between Each Step in Terms of Frame Frequency for the Second Screen
//    j: Screen Saver Automatic Sleep
//       "0x00" (Disable)
//       "0x01" (First Screen Only)
//       "0x02" (Second Screen Only)
//       "0x03" (First & Second Screen Both)
//    k: Set Sleep Timer in Terms of 64 Frames Synchronization
//    l: Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Screen_Saver_Control(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e, unsigned char f, unsigned char g, unsigned char h, unsigned char i, unsigned char j, unsigned char k, unsigned char l)
{
    Write_Command(0x3E);            // Update Timer Based on Frame Synchronization (SS_SCR1_FU)
    Write_Data(e);              //   Default => 0x00

    Write_Command(0x3F);            // Moving Step (SS_SCR1_MXY)
    Write_Data(d);              //   Default => 0x00
                        //     D[3:0] => Horizontal Moving Step

    Write_Command(0x40);            // Update Timer Based on Frame Synchronization (SS_SCR2_FU)
    Write_Data(i);              //   Default => 0x00

    Write_Command(0x41);            // Moving Step (SS_SCR2_MXY)
    Write_Data(g);              //   Default => 0x00
                        //     D[3:0] => Horizontal Moving Step
                        //     D[7:4] => Vertical Moving Step

    Write_Command(0x42);            // Screen Moving Direction (MOVING_DIRECTION)
    Write_Data((h<<4)&0x30);        //   Default => 0x00

    Write_Command(0x3C);            // Screen Saver Sleep Timer Based on 64 Frames Synchronization (SS_SLEEP_TIMER)
    Write_Data(k);              //   Default => 0x00

    Write_Command(0x3D);            // Screen Saver Control (SCREEN_SAVER_MODE)
    Write_Data(((f<<4)&0x30)|c);        //   Default => 0x00

    if(b < 2)
    {
        Set_Display_Mode(0x00);
    }
    else
    {
        Set_Display_Mode(0x04);
    }

    Write_Command(0x3B);            // Screen Saver Control (SCREEN_SAVER_CONTEROL)
    Write_Data(((j<<5)&0x60)|((b<<2)&0x0C)|a);
                        //   Default => 0x00
    for(i=0;i<l;i++) {
        SEPS525_DELAY_US(1);
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade In (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_In()
{
unsigned int i;

    Set_Display_On_Off(0x01);
    Set_Power_Save(0x04);
    for(i=0;i<200;i++)
    {
        SEPS525_DELAY_US(200);
    }
    Set_Power_Save(0x00);
    for(i=0;i<200;i++)
    {
        SEPS525_DELAY_US(200);
    }
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade Out (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Out()
{
unsigned int i;

    Set_Power_Save(0x00);
    for(i=0;i<200;i++)
    {
        SEPS525_DELAY_US(200);
    }
    Set_Power_Save(0x04);
    for(i=0;i<200;i++)
    {
        SEPS525_DELAY_US(200);
    }
    Set_Display_On_Off(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Sleep Mode
//
//    "0x00" Enter Sleep Mode
//    "0x01" Exit Sleep Mode
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Sleep(unsigned char a)
{
    switch(a)
    {
        case 0:
            Set_Display_On_Off(0x00);
            Set_Display_Mode(0x02);
            break;
        case 1:
            Set_Display_Mode(0x00);
            Set_Display_On_Off(0x01);
            break;
    }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Gray Scale Table Setting (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Gray_Scale_Table()
{
    Write_Command(0x50);
    Write_Data(0x00);           // Gray Scale Level 1 of Red
    Write_Command(0x51);
    Write_Data(0x01);
    Write_Command(0x50);
    Write_Data(0x01);           // Gray Scale Level 3 of Red
    Write_Command(0x51);
    Write_Data(0x0A);
    Write_Command(0x50);
    Write_Data(0x02);           // Gray Scale Level 5 of Red
    Write_Command(0x51);
    Write_Data(0x10);
    Write_Command(0x50);
    Write_Data(0x03);           // Gray Scale Level 7 of Red
    Write_Command(0x51);
    Write_Data(0x15);
    Write_Command(0x50);
    Write_Data(0x04);           // Gray Scale Level 9 of Red
    Write_Command(0x51);
    Write_Data(0x19);
    Write_Command(0x50);
    Write_Data(0x05);           // Gray Scale Level 11 of Red
    Write_Command(0x51);
    Write_Data(0x1E);
    Write_Command(0x50);
    Write_Data(0x06);           // Gray Scale Level 13 of Red
    Write_Command(0x51);
    Write_Data(0x23);
    Write_Command(0x50);
    Write_Data(0x07);           // Gray Scale Level 15 of Red
    Write_Command(0x51);
    Write_Data(0x28);
    Write_Command(0x50);
    Write_Data(0x08);           // Gray Scale Level 17 of Red
    Write_Command(0x51);
    Write_Data(0x2D);
    Write_Command(0x50);
    Write_Data(0x09);           // Gray Scale Level 19 of Red
    Write_Command(0x51);
    Write_Data(0x33);
    Write_Command(0x50);
    Write_Data(0x0A);           // Gray Scale Level 21 of Red
    Write_Command(0x51);
    Write_Data(0x38);
    Write_Command(0x50);
    Write_Data(0x0B);           // Gray Scale Level 23 of Red
    Write_Command(0x51);
    Write_Data(0x3D);
    Write_Command(0x50);
    Write_Data(0x0C);           // Gray Scale Level 25 of Red
    Write_Command(0x51);
    Write_Data(0x43);
    Write_Command(0x50);
    Write_Data(0x0D);           // Gray Scale Level 27 of Red
    Write_Command(0x51);
    Write_Data(0x47);
    Write_Command(0x50);
    Write_Data(0x0E);           // Gray Scale Level 29 of Red
    Write_Command(0x51);
    Write_Data(0x4D);
    Write_Command(0x50);
    Write_Data(0x0F);           // Gray Scale Level 31 of Red
    Write_Command(0x51);
    Write_Data(0x52);
    Write_Command(0x50);
    Write_Data(0x10);           // Gray Scale Level 33 of Red
    Write_Command(0x51);
    Write_Data(0x57);
    Write_Command(0x50);
    Write_Data(0x11);           // Gray Scale Level 35 of Red
    Write_Command(0x51);
    Write_Data(0x5C);
    Write_Command(0x50);
    Write_Data(0x12);           // Gray Scale Level 37 of Red
    Write_Command(0x51);
    Write_Data(0x60);
    Write_Command(0x50);
    Write_Data(0x13);           // Gray Scale Level 39 of Red
    Write_Command(0x51);
    Write_Data(0x64);
    Write_Command(0x50);
    Write_Data(0x14);           // Gray Scale Level 41 of Red
    Write_Command(0x51);
    Write_Data(0x69);
    Write_Command(0x50);
    Write_Data(0x15);           // Gray Scale Level 43 of Red
    Write_Command(0x51);
    Write_Data(0x6B);
    Write_Command(0x50);
    Write_Data(0x16);           // Gray Scale Level 45 of Red
    Write_Command(0x51);
    Write_Data(0x6F);
    Write_Command(0x50);
    Write_Data(0x17);           // Gray Scale Level 47 of Red
    Write_Command(0x51);
    Write_Data(0x73);
    Write_Command(0x50);
    Write_Data(0x18);           // Gray Scale Level 49 of Red
    Write_Command(0x51);
    Write_Data(0x75);
    Write_Command(0x50);
    Write_Data(0x19);           // Gray Scale Level 51 of Red
    Write_Command(0x51);
    Write_Data(0x76);
    Write_Command(0x50);
    Write_Data(0x1A);           // Gray Scale Level 53 of Red
    Write_Command(0x51);
    Write_Data(0x77);
    Write_Command(0x50);
    Write_Data(0x1B);           // Gray Scale Level 55 of Red
    Write_Command(0x51);
    Write_Data(0x78);
    Write_Command(0x50);
    Write_Data(0x1C);           // Gray Scale Level 57 of Red
    Write_Command(0x51);
    Write_Data(0x7A);
    Write_Command(0x50);
    Write_Data(0x1D);           // Gray Scale Level 59 of Red
    Write_Command(0x51);
    Write_Data(0x7B);
    Write_Command(0x50);
    Write_Data(0x1E);           // Gray Scale Level 61 of Red
    Write_Command(0x51);
    Write_Data(0x7C);
    Write_Command(0x50);
    Write_Data(0x1F);           // Gray Scale Level 63 of Red
    Write_Command(0x51);
    Write_Data(0x7D);
    Write_Command(0x50);
    Write_Data(0x20);           // Gray Scale Level 1 of Green
    Write_Command(0x51);
    Write_Data(0x01);
    Write_Command(0x50);
    Write_Data(0x21);           // Gray Scale Level 3 of Green
    Write_Command(0x51);
    Write_Data(0x0F);
    Write_Command(0x50);
    Write_Data(0x22);           // Gray Scale Level 5 of Green
    Write_Command(0x51);
    Write_Data(0x16);
    Write_Command(0x50);
    Write_Data(0x23);           // Gray Scale Level 7 of Green
    Write_Command(0x51);
    Write_Data(0x1C);
    Write_Command(0x50);
    Write_Data(0x24);           // Gray Scale Level 9 of Green
    Write_Command(0x51);
    Write_Data(0x22);
    Write_Command(0x50);
    Write_Data(0x25);           // Gray Scale Level 11 of Green
    Write_Command(0x51);
    Write_Data(0x27);
    Write_Command(0x50);
    Write_Data(0x26);           // Gray Scale Level 13 of Green
    Write_Command(0x51);
    Write_Data(0x2C);
    Write_Command(0x50);
    Write_Data(0x27);           // Gray Scale Level 15 of Green
    Write_Command(0x51);
    Write_Data(0x32);
    Write_Command(0x50);
    Write_Data(0x28);           // Gray Scale Level 17 of Green
    Write_Command(0x51);
    Write_Data(0x37);
    Write_Command(0x50);
    Write_Data(0x29);           // Gray Scale Level 19 of Green
    Write_Command(0x51);
    Write_Data(0x3C);
    Write_Command(0x50);
    Write_Data(0x2A);           // Gray Scale Level 21 of Green
    Write_Command(0x51);
    Write_Data(0x41);
    Write_Command(0x50);
    Write_Data(0x2B);           // Gray Scale Level 23 of Green
    Write_Command(0x51);
    Write_Data(0x46);
    Write_Command(0x50);
    Write_Data(0x2C);           // Gray Scale Level 25 of Green
    Write_Command(0x51);
    Write_Data(0x4A);
    Write_Command(0x50);
    Write_Data(0x2D);           // Gray Scale Level 27 of Green
    Write_Command(0x51);
    Write_Data(0x4F);
    Write_Command(0x50);
    Write_Data(0x2E);           // Gray Scale Level 29 of Green
    Write_Command(0x51);
    Write_Data(0x53);
    Write_Command(0x50);
    Write_Data(0x2F);           // Gray Scale Level 31 of Green
    Write_Command(0x51);
    Write_Data(0x58);
    Write_Command(0x50);
    Write_Data(0x30);           // Gray Scale Level 33 of Green
    Write_Command(0x51);
    Write_Data(0x5B);
    Write_Command(0x50);
    Write_Data(0x31);           // Gray Scale Level 35 of Green
    Write_Command(0x51);
    Write_Data(0x60);
    Write_Command(0x50);
    Write_Data(0x32);           // Gray Scale Level 37 of Green
    Write_Command(0x51);
    Write_Data(0x64);
    Write_Command(0x50);
    Write_Data(0x33);           // Gray Scale Level 39 of Green
    Write_Command(0x51);
    Write_Data(0x67);
    Write_Command(0x50);
    Write_Data(0x34);           // Gray Scale Level 41 of Green
    Write_Command(0x51);
    Write_Data(0x6A);
    Write_Command(0x50);
    Write_Data(0x35);           // Gray Scale Level 43 of Green
    Write_Command(0x51);
    Write_Data(0x6D);
    Write_Command(0x50);
    Write_Data(0x36);           // Gray Scale Level 45 of Green
    Write_Command(0x51);
    Write_Data(0x6F);
    Write_Command(0x50);
    Write_Data(0x37);           // Gray Scale Level 47 of Green
    Write_Command(0x51);
    Write_Data(0x71);
    Write_Command(0x50);
    Write_Data(0x38);           // Gray Scale Level 49 of Green
    Write_Command(0x51);
    Write_Data(0x74);
    Write_Command(0x50);
    Write_Data(0x39);           // Gray Scale Level 51 of Green
    Write_Command(0x51);
    Write_Data(0x75);
    Write_Command(0x50);
    Write_Data(0x3A);           // Gray Scale Level 53 of Green
    Write_Command(0x51);
    Write_Data(0x77);
    Write_Command(0x50);
    Write_Data(0x3B);           // Gray Scale Level 55 of Green
    Write_Command(0x51);
    Write_Data(0x78);
    Write_Command(0x50);
    Write_Data(0x3C);           // Gray Scale Level 57 of Green
    Write_Command(0x51);
    Write_Data(0x79);
    Write_Command(0x50);
    Write_Data(0x3D);           // Gray Scale Level 59 of Green
    Write_Command(0x51);
    Write_Data(0x7A);
    Write_Command(0x50);
    Write_Data(0x3E);           // Gray Scale Level 61 of Green
    Write_Command(0x51);
    Write_Data(0x7B);
    Write_Command(0x50);
    Write_Data(0x3F);           // Gray Scale Level 63 of Green
    Write_Command(0x51);
    Write_Data(0x7D);
    Write_Command(0x50);
    Write_Data(0x40);           // Gray Scale Level 1 of Blue
    Write_Command(0x51);
    Write_Data(0x01);
    Write_Command(0x50);
    Write_Data(0x41);           // Gray Scale Level 3 of Blue
    Write_Command(0x51);
    Write_Data(0x21);
    Write_Command(0x50);
    Write_Data(0x42);           // Gray Scale Level 5 of Blue
    Write_Command(0x51);
    Write_Data(0x29);
    Write_Command(0x50);
    Write_Data(0x43);           // Gray Scale Level 7 of Blue
    Write_Command(0x51);
    Write_Data(0x30);
    Write_Command(0x50);
    Write_Data(0x44);           // Gray Scale Level 9 of Blue
    Write_Command(0x51);
    Write_Data(0x36);
    Write_Command(0x50);
    Write_Data(0x45);           // Gray Scale Level 11 of Blue
    Write_Command(0x51);
    Write_Data(0x3B);
    Write_Command(0x50);
    Write_Data(0x46);           // Gray Scale Level 13 of Blue
    Write_Command(0x51);
    Write_Data(0x41);
    Write_Command(0x50);
    Write_Data(0x47);           // Gray Scale Level 15 of Blue
    Write_Command(0x51);
    Write_Data(0x46);
    Write_Command(0x50);
    Write_Data(0x48);           // Gray Scale Level 17 of Blue
    Write_Command(0x51);
    Write_Data(0x4C);
    Write_Command(0x50);
    Write_Data(0x49);           // Gray Scale Level 19 of Blue
    Write_Command(0x51);
    Write_Data(0x51);
    Write_Command(0x50);
    Write_Data(0x4A);           // Gray Scale Level 21 of Blue
    Write_Command(0x51);
    Write_Data(0x55);
    Write_Command(0x50);
    Write_Data(0x4B);           // Gray Scale Level 23 of Blue
    Write_Command(0x51);
    Write_Data(0x5A);
    Write_Command(0x50);
    Write_Data(0x4C);           // Gray Scale Level 25 of Blue
    Write_Command(0x51);
    Write_Data(0x5E);
    Write_Command(0x50);
    Write_Data(0x4D);           // Gray Scale Level 27 of Blue
    Write_Command(0x51);
    Write_Data(0x62);
    Write_Command(0x50);
    Write_Data(0x4E);           // Gray Scale Level 29 of Blue
    Write_Command(0x51);
    Write_Data(0x65);
    Write_Command(0x50);
    Write_Data(0x4F);           // Gray Scale Level 31 of Blue
    Write_Command(0x51);
    Write_Data(0x69);
    Write_Command(0x50);
    Write_Data(0x50);           // Gray Scale Level 33 of Blue
    Write_Command(0x51);
    Write_Data(0x6B);
    Write_Command(0x50);
    Write_Data(0x51);           // Gray Scale Level 35 of Blue
    Write_Command(0x51);
    Write_Data(0x6E);
    Write_Command(0x50);
    Write_Data(0x52);           // Gray Scale Level 37 of Blue
    Write_Command(0x51);
    Write_Data(0x70);
    Write_Command(0x50);
    Write_Data(0x53);           // Gray Scale Level 39 of Blue
    Write_Command(0x51);
    Write_Data(0x72);
    Write_Command(0x50);
    Write_Data(0x54);           // Gray Scale Level 41 of Blue
    Write_Command(0x51);
    Write_Data(0x74);
    Write_Command(0x50);
    Write_Data(0x55);           // Gray Scale Level 43 of Blue
    Write_Command(0x51);
    Write_Data(0x75);
    Write_Command(0x50);
    Write_Data(0x56);           // Gray Scale Level 45 of Blue
    Write_Command(0x51);
    Write_Data(0x76);
    Write_Command(0x50);
    Write_Data(0x57);           // Gray Scale Level 47 of Blue
    Write_Command(0x51);
    Write_Data(0x77);
    Write_Command(0x50);
    Write_Data(0x58);           // Gray Scale Level 49 of Blue
    Write_Command(0x51);
    Write_Data(0x77);
    Write_Command(0x50);
    Write_Data(0x59);           // Gray Scale Level 51 of Blue
    Write_Command(0x51);
    Write_Data(0x78);
    Write_Command(0x50);
    Write_Data(0x5A);           // Gray Scale Level 53 of Blue
    Write_Command(0x51);
    Write_Data(0x79);
    Write_Command(0x50);
    Write_Data(0x5B);           // Gray Scale Level 55 of Blue
    Write_Command(0x51);
    Write_Data(0x7A);
    Write_Command(0x50);
    Write_Data(0x5C);           // Gray Scale Level 57 of Blue
    Write_Command(0x51);
    Write_Data(0x7B);
    Write_Command(0x50);
    Write_Data(0x5D);           // Gray Scale Level 59 of Blue
    Write_Command(0x51);
    Write_Data(0x7B);
    Write_Command(0x50);
    Write_Data(0x5E);           // Gray Scale Level 61 of Blue
    Write_Command(0x51);
    Write_Data(0x7C);
    Write_Command(0x50);
    Write_Data(0x5F);           // Gray Scale Level 63 of Blue
    Write_Command(0x51);
    Write_Data(0x7D);
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Initialization
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void seps525_init(spi_device_handle_t spi)
{
    spi_device = spi;

    gpio_set_direction(SEPS525_PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(SEPS525_PIN_NUM_RST, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(SEPS525_PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(SEPS525_PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    Set_Power_Save(0x01);           // Set Normal Driving Current
    SEPS525_DELAY_US(200);                //     Disable Oscillator Power Down
                        //     Enable Power Save Mode
    Set_Power_Save(0x00);           // Set Normal Driving Current
    SEPS525_DELAY_US(200);                //     Disable Oscillator Power Down
                        //     Disable Power Save Mode
    Software_Reset(0x00);           // Set All Internal Register Value as Normal Mode
    Set_Display_On_Off(0x00);       // Display Off (0x00/0x01)
    Set_Clock_Control(0x01);        // Set EXPORT1 Pin at Internal Clock
                        //     Oscillator operates with external resister.
                        //     Internal Oscillator On
    Set_Display_Clock(0x30);        // Set Clock as 90 Frames/Sec
    Set_Multiplex_Ratio(0x7F);      // 1/128 Duty (0x0F~0x7F)
    Set_Display_Offset(0x00,0x00);      // Shift Mapping RAM Counter
    Set_Start_Line(0x00);           // Set Mapping RAM Display Start Line (0x00~0x7F)
    Set_RGB_IF(0x31);           // Set MCU Interface Mode
    Set_RGB_POL(0x00);          // Set RGB Interface Polarity as Active Low
                        //     Dot Clock Polarity Sampled as Rising Edge
                        //     Disable Vertical Synchronization Output on VSYNCO Pin
    Set_Display_Mode(0x00);         // Set Color Sequence D[15:0]=[RRRRR:GGGGGG:BBBBB]
                        //     Alternative Gate Pin Configuration
                        //     Scan from G0 to G127
                        //     Column Address 0 Mapped to S0
                        //     One Screen Mode
                        //     Normal Display
    Set_Pixel_Format(0x66|((Color_Depth<<4)&0x70));
                        // Set Horizontal Address Increment
                        //     Vertical Address Increment
                        //     The data is continuously written horizontally.
                        //     Enable 8-bit Bus Interface
                        //     65,536 Colors Mode (0x66)
                        //     * 262,144 Colors Mode (0x76)
    Set_Driving_Current(0x2F,0x31,0x1E);    // Set Driving Current of Red
                        // Set Driving Current of Green
                        // Set Driving Current of Blue
    Set_Gray_Scale_Table();         // Set Pulse Width for Gamma Table
    Set_Precharge_Period(0x03,0x04,0x01);   // Set Pre-Charge Time of Red
                        // Set Pre-Charge Time of Green
                        // Set Pre-Charge Time of Blue
    Set_Precharge_Current(0x1A,0x19,0x0A);  // Set Pre-Charge Current of Red
                        // Set Pre-Charge Current of Green
                        // Set Pre-Charge Current of Blue
    Set_IREF(0x00);             // Set Reference Voltage Controlled by External Resister
    First_Screen(0x00,0x9F,0x00,0x7F);

    Fill_RAM(0x00,0x00);            // Clear Screen

    Set_Display_On_Off(0x01);       // Display On (0x00/0x01)
}
