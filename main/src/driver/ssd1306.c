#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#include "inc/driver/ssd1306.h"
#include "inc/system/fonts.h"

/*-------------------------------------------------------------------------------*/

// 4-Wire SPI
#define SSD1306_PIN_NUM_MISO 25
#define SSD1306_PIN_NUM_MOSI 23
#define SSD1306_PIN_NUM_CLK  19
#define SSD1306_PIN_NUM_CS   22

#define SSD1306_PIN_NUM_DC   21
#define SSD1306_PIN_NUM_RST  18

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} ssd1306_init_cmd_t;

spi_device_handle_t spi_device = NULL;

#define SSD1306_DELAY_US(X)     vTaskDelay(X / 10 / portTICK_RATE_MS)

//Send a command to the SSD1306. Uses spi_device_transmit, which waits until the transfer is complete.
void ssd1306_cmd(spi_device_handle_t spi, const uint8_t cmd)
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

//Send data to the SSD1306. Uses spi_device_transmit, which waits until the transfer is complete.
void ssd1306_data(spi_device_handle_t spi, const uint8_t *data, int len)
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
void ssd1306_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(SSD1306_PIN_NUM_DC, dc);
}

/*-------------------------------------------------------------------------------*/

static unsigned char s_chDispalyBuffer[128][8];

/**
  * @brief  Writes an byte to the display data ram or the command register
  *         
  * @param  chData: Data to be writen to the display data ram or the command register
  * @param chCmd:  
  *                           0: Writes to the command register
  *                           1: Writes to the display data ram
  * @retval None
**/
static void ssd1306_write_byte(unsigned char chData, unsigned char chCmd)
{
	if (chCmd == SSD1306_CMD) {
		ssd1306_cmd(spi_device, chData);
	} else {
		ssd1306_data(spi_device, &chData, 1);
	}
}   	  

/**
  * @brief  OLED turns on 
  *         
  * @param  None
  *         
  * @retval None
**/ 
void ssd1306_display_on(void)
{
	ssd1306_write_byte(0x8D, SSD1306_CMD);  
	ssd1306_write_byte(0x14, SSD1306_CMD);  
	ssd1306_write_byte(0xAF, SSD1306_CMD);  
}
   
/**
  * @brief  OLED turns off
  *         
  * @param  None
  *         
  * @retval  None
**/
void ssd1306_display_off(void)
{
	ssd1306_write_byte(0x8D, SSD1306_CMD);  
	ssd1306_write_byte(0x10, SSD1306_CMD); 
	ssd1306_write_byte(0xAE, SSD1306_CMD);  
}

/**
  * @brief  Refreshs the graphic ram
  *         
  * @param  None
  *         
  * @retval  None
**/

void ssd1306_refresh_gram(void)
{
	unsigned char i, j;
	
	for (i = 0; i < 8; i ++) {  
		ssd1306_write_byte(0xB0 + i, SSD1306_CMD);    
		ssd1306_write_byte(0x00, SSD1306_CMD);
		ssd1306_write_byte(0x10, SSD1306_CMD);
		for (j = 0; j < 128; j ++) {
			ssd1306_write_byte(s_chDispalyBuffer[j][i], SSD1306_DATA);
		}
	}   
}

/**
  * @brief  Draws a piont on the screen
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  chPoint: 0: the point turns off    1: the piont turns on 
  *         
  * @retval None
**/

void ssd1306_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned char chPoint)
{
    unsigned char chPos, chBx, chTemp = 0;

    if (chXpos > 127 || chYpos > 63) {
        return;
    }
    chPos = 7 - chYpos / 8; //
    chBx = chYpos % 8;
    chTemp = 1 << (7 - chBx);

    if (chPoint) {
        s_chDispalyBuffer[chXpos][chPos] |= chTemp;
    } else {
        s_chDispalyBuffer[chXpos][chPos] &=~chTemp;
    }
}

/**
  * @brief   Clears the screen
  *
  * @param  None
  *
  * @retval  None
**/

void ssd1306_clear_screen(unsigned char chFill)
{
	unsigned char i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 128; j++) {
		    s_chDispalyBuffer[j][i] = chFill;
		}
	}
}

/**
  * @brief  Fills a rectangle
  *         
  * @param  chXpos1: Specifies the X position 1 (X top left position)
  * @param  chYpos1: Specifies the Y position 1 (Y top left position)
  * @param  chXpos2: Specifies the X position 2 (X bottom right position)
  * @param  chYpos3: Specifies the Y position 2 (Y bottom right position)
  *         
  * @retval 
**/

void ssd1306_fill_screen(unsigned char chXpos1, unsigned char chYpos1, unsigned char chXpos2, unsigned char chYpos2, unsigned char chDot)
{  
	unsigned char chXpos, chYpos;
	
	for (chXpos = chXpos1; chXpos <= chXpos2; chXpos ++) {
		for (chYpos = chYpos1; chYpos <= chYpos2; chYpos ++) {
			ssd1306_draw_point(chXpos, chYpos, chDot);
		}
	}
}

/**
  * @brief Displays one character at the specified position    
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  chSize: 
  * @param  chMode
  * @retval 
**/
void ssd1306_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chSize, unsigned char chMode)
{      	
	unsigned char i, j;
	unsigned char chTemp, chYpos0 = chYpos;
	
	chChr = chChr - ' ';				   
    for (i = 0; i < chSize; i ++) {   
		if (chSize == FONT_1206) {
			if (chMode) {
				chTemp = c_chFont1206[chChr][i];
			} else {
				chTemp = ~c_chFont1206[chChr][i];
			}
		} else {
			if (chMode) {
				chTemp = c_chFont1608[chChr][i];
			} else {
				chTemp = ~c_chFont1608[chChr][i];
			}
		}
		
        for (j = 0; j < 8; j ++) {
			if (chTemp & 0x80) {
				ssd1306_draw_point(chXpos, chYpos, 1);
			} else {
				ssd1306_draw_point(chXpos, chYpos, 0);
			}
			chTemp <<= 1;
			chYpos ++;
			
			if ((chYpos - chYpos0) == chSize) {
				chYpos = chYpos0;
				chXpos ++;
				break;
			}
		}  	 
    } 
}
static unsigned long pow_int(unsigned char m, unsigned char n)
{
	unsigned long result = 1;
	while(n --) result *= m;    
	return result;
}	

void ssd1306_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen, unsigned char chSize)
{         	
	unsigned char i;
	unsigned char chTemp, chShow = 0;
	
	for(i = 0; i < chLen; i ++) {
		chTemp = (chNum / pow_int(10, chLen - i - 1)) % 10;
		if(chShow == 0 && i < (chLen - 1)) {
			if(chTemp == 0) {
				ssd1306_display_char(chXpos + (chSize / 2) * i, chYpos, ' ', chSize, 1);
				continue;
			} else {
				chShow = 1;
			}	 
		}
	 	ssd1306_display_char(chXpos + (chSize / 2) * i, chYpos, chTemp + '0', chSize, 1); 
	}
} 

/**
  * @brief  Displays a string on the screen
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  pchString: Pointer to a string to display on the screen 
  *         
  * @retval  None
**/
void ssd1306_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chSize, unsigned char chMode)
{
    while (*pchString != '\0') {       
        if (chXpos > (SSD1306_WIDTH - chSize / 2)) {
			chXpos = 0;
			chYpos += chSize;
			if (chYpos > (SSD1306_HEIGHT - chSize)) {
				chYpos = chXpos = 0;
				ssd1306_clear_screen(0x00);
			}
		}
		
        ssd1306_display_char(chXpos, chYpos, *pchString, chSize, chMode);
        chXpos += chSize / 2;
        pchString ++;
    }
}

void ssd1306_draw_1612char(unsigned char chXpos, unsigned char chYpos, unsigned char chChar)
{
	unsigned char i, j;
	unsigned char chTemp = 0, chYpos0 = chYpos, chMode = 0;

	for (i = 0; i < 32; i ++) {
		chTemp = c_chFont1612[chChar - 0x30][i];
		for (j = 0; j < 8; j ++) {
			chMode = chTemp & 0x80? 1 : 0; 
			ssd1306_draw_point(chXpos, chYpos, chMode);
			chTemp <<= 1;
			chYpos ++;
			if ((chYpos - chYpos0) == 16) {
				chYpos = chYpos0;
				chXpos ++;
				break;
			}
		}
	}
}

void ssd1306_draw_3216char(unsigned char chXpos, unsigned char chYpos, unsigned char chChar)
{
	unsigned char i, j;
	unsigned char chTemp = 0, chYpos0 = chYpos, chMode = 0;

	for (i = 0; i < 64; i ++) {
		chTemp = c_chFont3216[chChar - 0x30][i];
		for (j = 0; j < 8; j ++) {
			chMode = chTemp & 0x80? 1 : 0; 
			ssd1306_draw_point(chXpos, chYpos, chMode);
			chTemp <<= 1;
			chYpos ++;
			if ((chYpos - chYpos0) == 32) {
				chYpos = chYpos0;
				chXpos ++;
				break;
			}
		}
	}
}

void ssd1306_draw_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight)
{
	unsigned int i, j, byteWidth = (chWidth + 7) / 8;
	
    for(j = 0; j < chHeight; j ++){
        for(i = 0; i < chWidth; i ++ ) {
            if(*(pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                ssd1306_draw_point(chXpos + i, chYpos + j, 1);
            }
        }
    }
}

void ssd1306_init(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num=SSD1306_PIN_NUM_MISO,
        .mosi_io_num=SSD1306_PIN_NUM_MOSI,
        .sclk_io_num=SSD1306_PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=26000000,               //Clock out at 10 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=SSD1306_PIN_NUM_CS,               //CS pin
        .queue_size=1,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=ssd1306_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi_device);
    assert(ret==ESP_OK);

    gpio_set_direction(SSD1306_PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(SSD1306_PIN_NUM_RST, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(SSD1306_PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(SSD1306_PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    
	ssd1306_write_byte(0xAE, SSD1306_CMD);//--turn off oled panel
	ssd1306_write_byte(0x00, SSD1306_CMD);//---set low column address
	ssd1306_write_byte(0x10, SSD1306_CMD);//---set high column address
	ssd1306_write_byte(0x40, SSD1306_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	ssd1306_write_byte(0x81, SSD1306_CMD);//--set contrast control register
	ssd1306_write_byte(0xCF, SSD1306_CMD);// Set SEG Output Current Brightness
	ssd1306_write_byte(0xA1, SSD1306_CMD);//--Set SEG/Column Mapping     
	ssd1306_write_byte(0xC0, SSD1306_CMD);//Set COM/Row Scan Direction   
	ssd1306_write_byte(0xA6, SSD1306_CMD);//--set normal display
	ssd1306_write_byte(0xA8, SSD1306_CMD);//--set multiplex ratio(1 to 64)
	ssd1306_write_byte(0x3f, SSD1306_CMD);//--1/64 duty
	ssd1306_write_byte(0xD3, SSD1306_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	ssd1306_write_byte(0x00, SSD1306_CMD);//-not offset
	ssd1306_write_byte(0xd5, SSD1306_CMD);//--set display clock divide ratio/oscillator frequency
	ssd1306_write_byte(0x80, SSD1306_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	ssd1306_write_byte(0xD9, SSD1306_CMD);//--set pre-charge period
	ssd1306_write_byte(0xF1, SSD1306_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	ssd1306_write_byte(0xDA, SSD1306_CMD);//--set com pins hardware configuration
	ssd1306_write_byte(0x12, SSD1306_CMD);
	ssd1306_write_byte(0xDB, SSD1306_CMD);//--set vcomh
	ssd1306_write_byte(0x40, SSD1306_CMD);//Set VCOM Deselect Level
	ssd1306_write_byte(0x20, SSD1306_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	ssd1306_write_byte(0x02, SSD1306_CMD);//
	ssd1306_write_byte(0x8D, SSD1306_CMD);//--set Charge Pump enable/disable
	ssd1306_write_byte(0x14, SSD1306_CMD);//--set(0x10) disable
	ssd1306_write_byte(0xA4, SSD1306_CMD);// Disable Entire Display On (0xa4/0xa5)
	ssd1306_write_byte(0xA6, SSD1306_CMD);// Disable Inverse Display On (0xa6/a7) 
	ssd1306_write_byte(0xAF, SSD1306_CMD);//--turn on oled panel
	
	ssd1306_clear_screen(0x00);
	ssd1306_refresh_gram();
}
