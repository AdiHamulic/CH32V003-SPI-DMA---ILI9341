/*******************************************************	 


	Author: Adamir Hamulic
	Date: 2023
	
	
********************************************************/



#include "uart.h"


void uart_init(void)
{
	//Enable clock for PORTD and UART1
	RCC->APB2PCENR |=  RCC_IOPDEN | RCC_USART1EN;
	//Enable PD6(RX) and PD5(TX) - PD6(Floating input), PD5(Multiplexed Push-Pull ouput mode 50Mhz)
	GPIOD->CFGLR = (GPIOD->CFGLR & ~(GPIO_CFGLR_CNF6 | GPIO_CFGLR_MODE5 | GPIO_CFGLR_MODE6)) \
					| GPIO_CFGLR_CNF6_0 | GPIO_CFGLR_CNF5_1 | GPIO_CFGLR_MODE5_0 | GPIO_CFGLR_MODE5_1;
	//Enable TX and RX 
	USART1->CTLR1 = USART_CTLR1_RE | USART_CTLR1_TE;
	
	//UART1 Baud rate - 115200
	USART1->BRR = (((uint16_t)UART_BAUD_115200) << 4) & USART_BRR_DIV_Mantissa;
	USART1->BRR |= (((uint16_t)UART_BAUD_115200) >> 4) & USART_BRR_DIV_Fraction;
	
	//Enable USART1
	USART1->CTLR1 |= USART_CTLR1_UE;
}

void uart_send_ch(char data)
{
	while((USART1->STATR & USART_STATR_TC) != USART_STATR_TC) {};
	USART1->DATAR = data;	
}
void uart_send_str(char *data)
{	
	while(*data) {
		uart_send_ch(*data++);
	}
}

char uart_recv_ch(void)
{
	if ((USART1->STATR & USART_STATR_RXNE) == USART_STATR_RXNE) {
		return (char)(USART1->DATAR);
	}
	return 0;
}
