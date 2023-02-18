/*******************************************************	 


	Author: Adamir Hamulic
	Date: 2023
	
	
********************************************************/


#ifndef __UART_H
#define __UART_H

#include "ch32v00x.h"



#define FCLK				(48000000)	// APB2 bus
#define UART_BAUD_115200	(FCLK / (16 * 115200))
#define UART_BAUD_57600		(FCLK / (16 * 57600))
#define UART_BAUD_9600		(FCLK / (16 * 9600))

void uart_init(void);
void uart_send_ch(char data);
void uart_send_str(char *data);
char uart_recv_ch(void);


#endif	/* __UART_H */ 
