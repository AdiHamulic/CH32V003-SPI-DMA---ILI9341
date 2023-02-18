/*
*
*	Author: Adamir Hamulic
*	Date: 2023
*
*	Testing display ILI9341 with CH32V003 over SPI(24 Mhz) with DMA
*
*/



#include "ch32v00x.h"
#include "ch32v00x_it.h"
#include "delay.h"
#include "uart.h"
#include "spi.h"
#include "ili9341.h"
#include "colors.h"
#include "fonts.h"
#include "stdlib.h"





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




const uint16_t color_arr[16] = { BLACK, NAVY, DGREEN, DCYAN, MAROON, PURPLE,OLIVE, LGRAY, DGRAY, BLUE, CYAN, MAGENTA,YELLOW, ORANGE, GREENYELLOW, MATRIX_GREEN };

const char sample_text[] = "CH32V003 series is an industrial-grade general-purpose microcontroller designed based on QingKe RISC-V2A\
core, which supports 48MHz system main frequency in the product function. The series features wide voltage,\
single line debug, low-power consumption and ultra-small package. It provides commonly used peripheral\
functions, built-in 1 group of DMA..."; 





int main(void)
{
	delay_init();
	uart_init();
	ili9341_init();
    ili9341_rotate_display(ili9341_portrait_1);
	

    while(1) {
    
       	//Test text changing color
	 	ili9341_fill(BLACK);
	 	ili9341_puts(25,100,	"Fast text color", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_puts(25,130,	"changing - 16x", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_rectangle(10,80,230,180, WHITE);
	 	delay_ms(2000);
   		for (uint8_t a = 0; a < 16; a++) {
   			ili9341_puts(0,0,(char *)sample_text,&TM_Font_11x18, color_arr[a], BLACK);	
   		}
   		delay_ms(500);
   		   
    	//Test menu scroll
	 	ili9341_fill(BLACK);
	 	ili9341_puts(25,100,	"Fast menu scroll", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_puts(60,130,	"test - 5x", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_rectangle(10,80,230,180, WHITE);
	 	delay_ms(2000);
	 	ili9341_fill(BLACK);
	 	for (uint8_t k = 0; k < 5; k++) {
			for (uint8_t j = 0; j < 7; j++) {
				ili9341_puts(0,0,	"     MENU      ", &TM_Font_16x26, BLACK, WHITE);
				ili9341_puts(0,33,	" Memory       >", &TM_Font_16x26, j == 0 ? BLACK : WHITE, j == 0 ? WHITE : BLACK);
				ili9341_puts(0,68,	" Cal Settings >", &TM_Font_16x26, j == 1 ? BLACK : WHITE, j == 1 ? WHITE : BLACK);
				ili9341_puts(0,98,	" Setup        >", &TM_Font_16x26, j == 2 ? BLACK : WHITE, j == 2 ? WHITE : BLACK);
				ili9341_puts(0,128,	" Connect      >", &TM_Font_16x26, j == 3 ? BLACK : WHITE, j == 3 ? WHITE : BLACK);
				ili9341_puts(0,158,	" Help         ",  &TM_Font_16x26, j == 4 ? BLACK : WHITE, j == 4 ? WHITE : BLACK);
				ili9341_puts(0,188,	" Power Off    ",  &TM_Font_16x26, j == 5 ? BLACK : WHITE, j == 5 ? WHITE : BLACK);
				ili9341_puts(0,218,	" Exit         ",  &TM_Font_16x26, j == 6 ? BLACK : WHITE, j == 6 ? WHITE : BLACK); 
			}
		}
  		delay_ms(500);
		
	  	//Test screen fill	
	 	ili9341_fill(BLACK);
	 	ili9341_puts(25,100,	"Fill screen test", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_puts(100,130,	"48x", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_rectangle(10,80,230,180, WHITE);
	 	delay_ms(2000);
	 	for (uint8_t i = 0; i < 3; i++) {
	   		for (uint8_t m = 0; m < 16; m++) {
	   			ili9341_fill(color_arr[m]);		
	   		}	 	
	 	}  	 	 		
  		delay_ms(500);
  		
   		//Filled random rectangles 40x
   	 	ili9341_fill(BLACK);
	 	ili9341_puts(20,100,	"Random color fill", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_puts(20,130,	"rectangle test 40x", &TM_Font_11x18, WHITE, BLACK);
	 	ili9341_rectangle(10,80,230,180, WHITE);
	 	delay_ms(2000); 
   		for (uint8_t i = 0; i < 40; i++) {
    		for (uint16_t n = 0; n <= 320; n += 32) {
	   			for (uint16_t j = 0; j <= 240; j += 16) {
	   				ili9341_fill_rectangle(j,n,j+16,n+32,color_arr[rand()%16]);
	   				ili9341_rectangle(j,n,j+16,n+32,BLACK);
	   			}
   			}  			
   		}
   		delay_ms(500);
   		
	}
}

