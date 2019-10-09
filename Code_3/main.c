/*
 * main.c
 *
 * Created: 4-6-2019 19:09:01
 *  Author: Gebruiker
 */ 

#define F_CPU 32000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "clock.h"


int main(void)
{
	Config32MHzClock();
	
	ws2812_init();
	
	//timer that executes interrupt
	TCF0.CTRLB		= TC_WGMODE_NORMAL_gc;
	TCF0.INTCTRLA	= TC_OVFINTLVL_LO_gc;
	TCF0.PER		= (uint16_t)(F_CPU / 60 / 64 - 1);		//PER register sets how far the timer will count
	TCF0.CTRLA		= TC_CLKSEL_DIV64_gc;
	
	PMIC.CTRL		|= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
	
	showColor(PIXELS, 0, 0, 0xBB); 
	
	while (1)
	{
		asm volatile ("nop");
	}
}