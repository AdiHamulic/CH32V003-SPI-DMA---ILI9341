/*******************************************************	 


	Author: Adamir Hamulic
	Date: 2023
	
	
********************************************************/


#ifndef	__SPI_H
#define	__SPI_H

#include "ch32v00x.h"



#define SPI_DATA_8B()			(SPI1->CTLR1 &= ~SPI_CTLR1_DFF)
#define SPI_DATA_16B()			(SPI1->CTLR1 |= SPI_CTLR1_DFF)
#define SPI_DMA_MEM_INC_ON()	(DMA1_Channel3->CFGR |= DMA_CFGR1_MINC)
#define SPI_DMA_MEM_INC_OFF()	(DMA1_Channel3->CFGR &= ~DMA_CFGR1_MINC)



void spi_init(void);
void spi_send8(uint8_t data);
void spi_send16(uint16_t data);
uint8_t spi_recv8(uint8_t dummy);
void spi_send_dma16(uint16_t *data, uint16_t size);



#endif	/* __SPI_H */
