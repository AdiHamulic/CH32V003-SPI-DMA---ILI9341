/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
 
 
 
/*
*	Author: Adamir Hamulic
*	Date: 2023
*	Added support for WCH chip CH32V003 
*	with using DMA for SPI in functions putc, fill_rectangle, fill_display...
*
*/








#include "ili9341.h"
#include "delay.h"
#include "fonts.h"
#include "colors.h"





/********************************************
	LCD 2.8 240*320 ILI9341 - with CS pin
	Pins	
	1. MISO -> PC7
	2. LED  -> PC3
	3. SCK 	-> PC5
	4. MOSI -> PC6
	5. D/C	-> PC2
	6. RST	-> PC1
	7. CS	-> PC4
	8. GND
	9. VCC
**********************************************/


//DMA buffer for putc function
uint16_t buffer[512];




#define ILI9341_RST_ON()	(GPIOC->BSHR = GPIO_BSHR_BR1)
#define ILI9341_RST_OFF()	(GPIOC->BSHR = GPIO_BSHR_BS1)

#define ILI9341_DC_DATA()	(GPIOC->BSHR = GPIO_BSHR_BS2)
#define ILI9341_DC_CMD()	(GPIOC->BSHR = GPIO_BSHR_BR2)

#define ILI9341_LED_ON()	(GPIOC->BSHR = GPIO_BSHR_BS3)
#define ILI9341_LED_OFF()	(GPIOC->BSHR = GPIO_BSHR_BR3)

#define ILI9341_CS_ON()		(GPIOC->BSHR = GPIO_BSHR_BR4)
#define ILI9341_CS_OFF()	(GPIOC->BSHR = GPIO_BSHR_BS4)



#define ILI9341_RESET				0x01
#define ILI9341_SLEEP_OUT			0x11
#define ILI9341_GAMMA				0x26
#define ILI9341_DISPLAY_OFF			0x28
#define ILI9341_DISPLAY_ON			0x29
#define ILI9341_COLUMN_ADDR			0x2A
#define ILI9341_PAGE_ADDR			0x2B
#define ILI9341_GRAM				0x2C
#define ILI9341_MAC					0x36
#define ILI9341_PIXEL_FORMAT		0x3A
#define ILI9341_WRITE_CONTINUE		0x3C
#define ILI9341_WDB					0x51
#define ILI9341_WCD					0x53
#define ILI9341_RGB_INTERFACE		0xB0
#define ILI9341_FRC					0xB1
#define ILI9341_BPC					0xB5
#define ILI9341_DFC					0xB6
#define ILI9341_POWER1				0xC0
#define ILI9341_POWER2				0xC1
#define ILI9341_VCOM1				0xC5
#define ILI9341_VCOM2				0xC7
#define ILI9341_POWERA				0xCB
#define ILI9341_POWERB				0xCF
#define ILI9341_PGAMMA				0xE0
#define ILI9341_NGAMMA				0xE1
#define ILI9341_DTCA				0xE8
#define ILI9341_DTCB				0xEA
#define ILI9341_POWER_SEQ			0xED
#define ILI9341_3GAMMA_EN			0xF2
#define ILI9341_INTERFACE			0xF6
#define ILI9341_PRC					0xF7


typedef enum {
	ili9341_landscape = 0,
	ili9341_portrait
} ili9341_orient_mode_t;

typedef struct {
	uint16_t width;
	uint16_t height;
	ili9341_orient_mode_t lcd_orientation;
} ili9341_t;

uint16_t ili9341_x;
uint16_t ili9341_y;
ili9341_t ILI9341;




static void ili9341_send_data8(uint8_t data)
{
	SPI_DATA_8B();
	ILI9341_DC_DATA();
	ILI9341_CS_ON();
	spi_send8(data);
	ILI9341_CS_OFF();
}

static void ili9341_send_cmd8(uint8_t cmd)
{
	SPI_DATA_8B();
	ILI9341_DC_CMD();
	ILI9341_CS_ON();
	spi_send8(cmd);
	ILI9341_CS_OFF();
}

static void ili9341_send_data16(uint16_t data)
{
	SPI_DATA_16B();
	ILI9341_DC_DATA();
	ILI9341_CS_ON();
	spi_send16(data);
	ILI9341_CS_OFF();
}

static void ili9341_send_dma_data16(uint16_t *data, uint16_t size)
{
	ILI9341_DC_DATA();
	ILI9341_CS_ON();
	//Send data
	spi_send_dma16(data, size);
	//Wait end of transfer
	while((DMA1->INTFR & DMA_TCIF3) != DMA_TCIF3){};
	//Clear DMA global flag  
	DMA1->INTFCR |= DMA_CGIF3;
}

static void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	ili9341_send_cmd8(ILI9341_COLUMN_ADDR);
	ili9341_send_data16(x0);
	ili9341_send_data16(x1);
	ili9341_send_cmd8(ILI9341_PAGE_ADDR);
	ili9341_send_data16(y0);
	ili9341_send_data16(y1);
	ili9341_send_cmd8(ILI9341_GRAM);
}



void ili9341_init(void)
{
	//Init variables
	ili9341_x = 0;
	ili9341_y = 0;
	ILI9341.width = ILI9341_WIDTH;
	ILI9341.height = ILI9341_HEIGHT;
	ILI9341.lcd_orientation = ili9341_portrait;
	
	/* Init SPI1, DMA */
	spi_init();
	
	/* Init GPIO pins */
	//Enable clock for PORTC
	RCC->APB2PCENR |=  RCC_IOPCEN;
	//Set PC1,PC2, PC3 and PC4 as outputs, at max speed - 50Mhz
	GPIOC->CFGLR = (GPIOC->CFGLR & ~(GPIO_CFGLR_CNF1 | GPIO_CFGLR_CNF2 | GPIO_CFGLR_CNF3 | GPIO_CFGLR_CNF4 \
				| GPIO_CFGLR_MODE1 | GPIO_CFGLR_MODE2 | GPIO_CFGLR_MODE3 | GPIO_CFGLR_MODE4)) \
				| GPIO_CFGLR_MODE1_0 | GPIO_CFGLR_MODE1_1 | GPIO_CFGLR_MODE2_0 | GPIO_CFGLR_MODE2_1 \
				| GPIO_CFGLR_MODE3_0 | GPIO_CFGLR_MODE3_1 | GPIO_CFGLR_MODE4_0 | GPIO_CFGLR_MODE4_1;
	
	//Reset LCD
	ILI9341_RST_ON();
	delay_ms(1);
	ILI9341_RST_OFF();
	delay_ms(10);
	ili9341_send_cmd8(ILI9341_RESET);
	delay_ms(100);
	//End reset
	
	ili9341_send_cmd8(ILI9341_POWERA);
	ili9341_send_data8(0x39);
	ili9341_send_data8(0x2C);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x34);
	ili9341_send_data8(0x02);
	
	ili9341_send_cmd8(ILI9341_POWERB);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0xC1);
	ili9341_send_data8(0x30);

	ili9341_send_cmd8(ILI9341_DTCA);
	ili9341_send_data8(0x85);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x78); 
	
	ili9341_send_cmd8(ILI9341_DTCB);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x00);
	
	ili9341_send_cmd8(ILI9341_POWER_SEQ);
	ili9341_send_data8(0x64);
	ili9341_send_data8(0x03);
	ili9341_send_data8(0x12);
	ili9341_send_data8(0x81);
	
	ili9341_send_cmd8(ILI9341_PRC);
	ili9341_send_data8(0x20);
	
	ili9341_send_cmd8(ILI9341_POWER1);
	ili9341_send_data8(0x23);
	
	ili9341_send_cmd8(ILI9341_POWER2);
	ili9341_send_data8(0x10);
	
	ili9341_send_cmd8(ILI9341_VCOM1);
	ili9341_send_data8(0x3E);
	ili9341_send_data8(0x28);
	
	ili9341_send_cmd8(ILI9341_VCOM2);
	ili9341_send_data8(0x86);
	
	ili9341_send_cmd8(ILI9341_MAC);
	ili9341_send_data8(0x48);
	
	ili9341_send_cmd8(ILI9341_PIXEL_FORMAT);
	ili9341_send_data8(0x55);
	
	ili9341_send_cmd8(ILI9341_FRC);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x13);
	
	ili9341_send_cmd8(ILI9341_DFC);
	ili9341_send_data8(0x08);
	ili9341_send_data8(0x82);
	ili9341_send_data8(0x27);
	
	ili9341_send_cmd8(ILI9341_3GAMMA_EN);
	ili9341_send_data8(0x00);
	
	ili9341_send_cmd8(ILI9341_COLUMN_ADDR);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0xEF);
	
	ili9341_send_cmd8(ILI9341_PAGE_ADDR);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x01);
	ili9341_send_data8(0x3F);
	
	ili9341_send_cmd8(ILI9341_GAMMA);
	ili9341_send_data8(0x01);
	
	ili9341_send_cmd8(ILI9341_PGAMMA);
	ili9341_send_data8(0x0F);
	ili9341_send_data8(0x31);
	ili9341_send_data8(0x2B);
	ili9341_send_data8(0x0C);
	ili9341_send_data8(0x0E);
	ili9341_send_data8(0x08);
	ili9341_send_data8(0x4E);
	ili9341_send_data8(0xF1);
	ili9341_send_data8(0x37);
	ili9341_send_data8(0x07);
	ili9341_send_data8(0x10);
	ili9341_send_data8(0x03);
	ili9341_send_data8(0x0E);
	ili9341_send_data8(0x09);
	ili9341_send_data8(0x00);
	
	ili9341_send_cmd8(ILI9341_NGAMMA);
	ili9341_send_data8(0x00);
	ili9341_send_data8(0x0E);
	ili9341_send_data8(0x14);
	ili9341_send_data8(0x03);
	ili9341_send_data8(0x11);
	ili9341_send_data8(0x07);
	ili9341_send_data8(0x31);
	ili9341_send_data8(0xC1);
	ili9341_send_data8(0x48);
	ili9341_send_data8(0x08);
	ili9341_send_data8(0x0F);
	ili9341_send_data8(0x0C);
	ili9341_send_data8(0x31);
	ili9341_send_data8(0x36);
	ili9341_send_data8(0x0F);
	
	ili9341_send_cmd8(ILI9341_SLEEP_OUT);
	delay_ms(10);
	ili9341_send_cmd8(ILI9341_DISPLAY_ON);
	delay_ms(10);
	ili9341_send_cmd8(ILI9341_GRAM);	
	ILI9341_LED_ON();
}

void ili9341_cursor_position(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
	ili9341_send_cmd8(ILI9341_COLUMN_ADDR);
	ili9341_send_data16(x1);
	ili9341_send_data16(x2);
	ili9341_send_cmd8(ILI9341_PAGE_ADDR);
	ili9341_send_data16(y1);
	ili9341_send_data16(y2);
}

void ili9341_fill(uint16_t color)
{
	ili9341_set_window(0,0,ILI9341.width-1,ILI9341.height-1);
	ili9341_send_dma_data16(&color, (uint16_t)(UINT16_MAX));
	ili9341_send_dma_data16(&color, (uint16_t)((ILI9341_WIDTH * ILI9341_HEIGHT)  - UINT16_MAX));
}


void ili9341_v_line(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t color)
{
	if (x1 >= x0){
		ili9341_cursor_position(x0, y0, x1+1, y0);
		ili9341_send_cmd8(ILI9341_GRAM);
		ili9341_send_cmd8(ILI9341_WRITE_CONTINUE);
		ili9341_send_dma_data16(&color ,(x1-x0));
	}
}

void ili9341_h_line(uint16_t y0,uint16_t y1, uint16_t x0, uint16_t color)
{
	if (y1 >= y0){
		ili9341_cursor_position(x0, y0, x0, y1+1);
		ili9341_send_cmd8(ILI9341_GRAM);
		ili9341_send_cmd8(ILI9341_WRITE_CONTINUE);
		ili9341_send_dma_data16(&color ,(y1-y0));
	}
}

void ili9341_fill_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	
	ili9341_cursor_position(x0, y0, x1-1, y1-1);
	ili9341_send_cmd8(ILI9341_GRAM);
	ili9341_send_cmd8(ILI9341_WRITE_CONTINUE);
	ili9341_send_dma_data16(&color ,(x1-x0)*(y1-y0));
}

void ili9341_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) 
{
	 ili9341_v_line(x0, x1, y0, color);
	 ili9341_h_line(y0, y1, x0, color);
	 ili9341_h_line(y0, y1+1, x1, color);
	 ili9341_v_line(x0, x1, y1, color);
}

void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color) 
{
	ili9341_cursor_position(x, y, x, y);
	ili9341_send_cmd8(ILI9341_GRAM);
	ili9341_send_data16(color);
}

void ili9341_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    ili9341_draw_pixel(x0, y0 + r, color);
    ili9341_draw_pixel(x0, y0 - r, color);
    ili9341_draw_pixel(x0 + r, y0, color);
    ili9341_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ili9341_draw_pixel(x0 + x, y0 + y, color);
        ili9341_draw_pixel(x0 - x, y0 + y, color);
        ili9341_draw_pixel(x0 + x, y0 - y, color);
        ili9341_draw_pixel(x0 - x, y0 - y, color);

        ili9341_draw_pixel(x0 + y, y0 + x, color);
        ili9341_draw_pixel(x0 - y, y0 + x, color);
        ili9341_draw_pixel(x0 + y, y0 - x, color);
        ili9341_draw_pixel(x0 - y, y0 - x, color);
    }
}

void ili9341_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    ili9341_draw_pixel(x0, y0 + r, color);
    ili9341_draw_pixel(x0, y0 - r, color);
    ili9341_draw_pixel(x0 + r, y0, color);
    ili9341_draw_pixel(x0 - r, y0, color);
    ili9341_v_line(x0 - r, x0 + r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ili9341_v_line(x0 - x, x0 + x, y0 + y, color);
        ili9341_v_line(x0 - x, x0 + x, y0 - y, color);
        ili9341_v_line(x0 - y, x0 + y, y0 + x, color);
        ili9341_v_line(x0 - y, x0 + y, y0 - x, color);

    }
}

void ili9341_rotate_display(ili9341_orientation_t orientation)
{
	ili9341_send_cmd8(ILI9341_MAC);
	
	if (orientation == ili9341_portrait_1) {
		ili9341_send_data8(0x58);
	} else if (orientation == ili9341_portrait_2) {
		ili9341_send_data8(0x88);
	} else if (orientation == ili9341_landscape_1) {
		ili9341_send_data8(0x28);
	} else if (orientation == ili9341_landscape_2) {
		ili9341_send_data8(0xE8);
	}

	if (orientation == ili9341_portrait_1 || orientation == ili9341_portrait_2) {
		ILI9341.width = ILI9341_WIDTH;
		ILI9341.height = ILI9341_HEIGHT;
		ILI9341.lcd_orientation = ili9341_landscape;
	} else {
		ILI9341.width = ILI9341_HEIGHT;
		ILI9341.height = ILI9341_WIDTH;
		ILI9341.lcd_orientation = ili9341_landscape;
	}
}

void ili9341_get_font_size(char *str, TM_FontDef_t *font, uint16_t *width, uint16_t *height) 
{
	uint16_t w = 0;
	*height = font->FontHeight;
	while (*str++) {
		w += font->FontWidth;
	}
	*width = w;
}

void ili9341_putc(uint16_t x, uint16_t y, char c, TM_FontDef_t *font, uint32_t foreground, uint32_t background) 
{
	uint32_t i, b, j;

    /* Set coordinates */
	ili9341_x = x;
	ili9341_y = y;
	if ((ili9341_x + font->FontWidth) > ILI9341.width) {
		//If at the end of a line of display, go to new line and set x to 0 position
		ili9341_y += font->FontHeight;
		ili9341_x = 0;
	}
	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];

		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				buffer[j+(i*font->FontWidth)] = foreground;
			} else if ((background & ILI9341_TRANSPARENT) == 0) {
				buffer[j+(i*font->FontWidth)] = background;
			}
		}
	}

	ili9341_cursor_position(ili9341_x,ili9341_y, (ili9341_x + font->FontWidth - 1), (ili9341_y+font->FontHeight - 1));
	ili9341_send_cmd8(ILI9341_GRAM);
	ili9341_send_cmd8(ILI9341_WRITE_CONTINUE);
	SPI_DMA_MEM_INC_ON();
	ili9341_send_dma_data16(buffer ,(font->FontWidth*font->FontHeight));
	SPI_DMA_MEM_INC_OFF();
	ili9341_x += font->FontWidth;
}

void ili9341_puts(uint16_t x, uint16_t y, char *str, TM_FontDef_t *font, uint32_t foreground, uint32_t background) 
{
	uint16_t startX = x;

	/* Set X and Y coordinates */
	ili9341_x = x;
	ili9341_y = y;

	while (*str) {
		//New line
		if (*str == '\n') {
			ili9341_y += font->FontHeight + 1;
			//if after \n is also \r, than go to the left of the screen
			if (*(str + 1) == '\r') {
				ili9341_x = 0;
				str++;
			} else {
				ili9341_x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}
		ili9341_putc(ili9341_x, ili9341_y, *str++, font, foreground, background);
	}
}

