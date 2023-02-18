/*******************************************************	 


	Author: Adamir Hamulic
	Date: 2023
	
	
********************************************************/



#include "spi.h"


void spi_init(void)
{
	//Enable clock for PORTC, SPI1 and DMA1
	RCC->APB2PCENR |=  RCC_IOPCEN | RCC_SPI1EN;
	RCC->AHBPCENR |= RCC_DMA1EN;
	
	//Enable PC7(MISO) - Floating input, PC6(MOSI) and PC5(SCK) - Multiplexed push pull output
	//GPIO max speed - 50Mhz
	GPIOC->CFGLR = (GPIOC->CFGLR & ~(GPIO_CFGLR_CNF7 | GPIO_CFGLR_CNF6 | GPIO_CFGLR_CNF5 \
					| GPIO_CFGLR_MODE6 | GPIO_CFGLR_MODE5)) \
					| GPIO_CFGLR_CNF7_0 | GPIO_CFGLR_CNF6_1 | GPIO_CFGLR_CNF5_1 \
					| GPIO_CFGLR_MODE6_0 | GPIO_CFGLR_MODE6_1 | GPIO_CFGLR_MODE5_0 | GPIO_CFGLR_MODE5_1;
	
	//Set SPI1, max clock 48Mhz/2 = 24Mhz, master mode, full-duplex mode,8bit data length
	//Internal slave select and software slave managment
	SPI1->CTLR1 = SPI_CTLR1_SSI | SPI_CTLR1_SSM | SPI_CTLR1_MSTR; //| SPI_CTLR1_BR_1 | SPI_CTLR1_BR_0;
	//Enable DMA for SPI TX
	SPI1->CTLR2 |= SPI_CTLR2_TXDMAEN;
	
	//Set DMA1 - Channel 3, Very high priority, 16bit size, mem -> spi direction
	//No memory or perpheral increment
	DMA1_Channel3->CFGR |= DMA_CFGR1_PL_0 | DMA_CFGR1_PL_1 | DMA_CFGR1_PSIZE_0 \
						| DMA_CFGR1_MSIZE_0 | DMA_CFGR1_DIR;					
	//Take SPI1 Data register address
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	//Enable DMA interrupt for channel 3
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);
	
	//Enable SPI1
	SPI1->CTLR1 |= SPI_CTLR1_SPE;
}

void spi_send8(uint8_t data)
{
	while((SPI1->STATR & SPI_STATR_TXE) != SPI_STATR_TXE){};
	SPI1->DATAR = data;
	while((SPI1->STATR & SPI_STATR_BSY) == SPI_STATR_BSY){};
}

void spi_send16(uint16_t data)
{
	while((SPI1->STATR & SPI_STATR_TXE) != SPI_STATR_TXE){};
	SPI1->DATAR = data;
	while((SPI1->STATR & SPI_STATR_BSY) == SPI_STATR_BSY){};
}

uint8_t spi_recv8(uint8_t dummy)
{
	SPI1->DATAR = dummy;
	while((SPI1->STATR & SPI_STATR_RXNE) != SPI_STATR_RXNE){};
	
	return (uint8_t)SPI1->DATAR;
}

void spi_send_dma16(uint16_t *data, uint16_t size)
{
	//Change data length to 16bit
	SPI_DATA_16B();
	//First disable DMA
	DMA1_Channel3->CFGR &= ~DMA_CFGR3_EN;
	//Buffer address
	DMA1_Channel3->MADDR = (uint32_t)data;
	//Number of data transfer
	DMA1_Channel3->CNTR = (uint16_t)size;
	//Enable DMA Channel
	DMA1_Channel3->CFGR |= DMA_CFGR3_EN;	
}
