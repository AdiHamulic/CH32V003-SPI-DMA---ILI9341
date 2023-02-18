/*******************************************************	 


	Author: Adamir Hamulic
	Date: 2023
	
	
********************************************************/



#include "delay.h"


volatile uint32_t delay_cnt;


void delay_init(void)
{
	//Enable SysTick interrupt
	NVIC_EnableIRQ(SysTicK_IRQn);
	//Set SysTick
	SysTick->SR &= ~(1 << 0);
    SysTick->CMP = (SystemCoreClock / 1000 ) - 1;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;

}

void delay_ms(uint32_t ms)
{
	delay_cnt = ms; 
	while(delay_cnt) {
		__NOP();
	}
}
