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


#ifndef __ILI9341_H
#define __ILI9341_H

#include "ch32v00x.h"
#include "spi.h"
#include "fonts.h"



#define ILI9341_WIDTH			240
#define ILI9341_HEIGHT			320

#define ILI9341_TRANSPARENT		0x80000000

typedef enum {
	ili9341_portrait_1 = 0,
	ili9341_portrait_2,
	ili9341_landscape_1,
	ili9341_landscape_2
} ili9341_orientation_t;



void ili9341_init(void);
void ili9341_fill(uint16_t color);
void ili9341_cursor_position(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ili9341_v_line(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t color);
void ili9341_h_line(uint16_t y0,uint16_t y1, uint16_t x0, uint16_t color);
void ili9341_fill_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ili9341_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void ili9341_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void ili9341_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void ili9341_rotate_display(ili9341_orientation_t orientation);
void ili9341_get_font_size(char *str, TM_FontDef_t *font, uint16_t *width, uint16_t *height);
void ili9341_putc(uint16_t x, uint16_t y, char c, TM_FontDef_t *font, uint32_t foreground, uint32_t background);
void ili9341_puts(uint16_t x, uint16_t y, char *str, TM_FontDef_t *font, uint32_t foreground, uint32_t background);





#endif /* __ILI9341_H */ 
