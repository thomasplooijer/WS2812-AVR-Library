/*
 * spi.c
 *
 * Created: 3-2-2018 14:03:57
 *  Author: Thomas Plooijer
 */ 
#include <avr/io.h>
#include "spi.h"


//spi standard settings
void spi_init(void)
{
	PORTD.DIRSET  =  SPI_SCK_bm|SPI_MOSI_bm|SPI_SS_bm;
	PORTD.DIRCLR  =  SPI_MISO_bm;
	PORTD.OUTSET  =  SPI_SS_bm;
	SPID.CTRL     =  SPI_ENABLE_bm |         // enable SPI
	SPI_MASTER_bm |         // master mode
	// SPI_CLK2X_bm  |         // no double clock speed (leave off)
	// SPI_DORD_bm   |         // MSB first (leave off)
	SPI_MODE_0_gc |         // SPI mode 0
	SPI_PRESCALER_DIV4_gc;  // SPI prescaling 4 (32Mhz/4 = 8Mhz. 1/8Mhz = 0.125us/bit)
}

uint8_t spi_transfer(uint8_t data)
{
	SPID.DATA = data;
	while(!(SPID.STATUS & (SPI_IF_bm)));

	return SPID.DATA;
}

void spi_write(uint8_t data)
{
	PORTD.OUTCLR = SPI_SS_bm;
	spi_transfer(data);
	PORTD.OUTSET = SPI_SS_bm;
}

uint8_t spi_read(void)
{
	uint8_t data;

	PORTD.OUTCLR = SPI_SS_bm;
	data = spi_transfer(FOO);
	PORTD.OUTSET = SPI_SS_bm;

	return data;
}