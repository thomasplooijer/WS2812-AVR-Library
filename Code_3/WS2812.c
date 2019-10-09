/*
 * Code_1.c
 *
 * Created: 6-2-2018 19:28:29
 * Author : Thomas Plooijer
 *
 * Using timer and PWM to produce correct timing for executing PWM dutycycle.
 * Timer PER is 40 because 32.000.000/800.000 (dataspeed) = 40
 * PWM PER (top) = 50 because we need 40//10 High//Low
 * 
 * Datapin: D8 (PE0) on AtXmega256A3U
 *
 * TO DO: Create arraysystem to light up individual pixels
 * 
 */ 

//cpu speed of 32MHz
#define F_CPU		32000000UL
#define numPIXELS	60
#define MASK		0x80
#define ANIMATION	8

//ADC defines
#define MAX_VALUE   4095
#define VCC         3.30
#define VREF        (((double) VCC) / 1.6)  // is 2.06125
#define VOFFSET     1.28	//-1,23V MicOUT bias

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "clock.h"

const unsigned int PIXELS = numPIXELS;                  // Pixels in the strip

uint8_t animation = ANIMATION;

uint8_t lookup[40] = {0, 20, 30, 40, 60, 70, 80, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 210, 200, 190
	, 180, 170, 160, 150, 140, 130, 120, 110, 100, 80, 70, 60, 40, 30, 20, 0};

uint8_t lookupTwo[7] = {0, 50, 80, 150, 80, 50, 0};

//routines
void ws2812_init();
void sendByte (uint8_t b);
void Config32MHzClock(void);
void sendPixel (uint8_t r, uint8_t g, uint8_t b);
void showColor (unsigned int count, unsigned char r , unsigned char g , unsigned char b);
void setPixel(uint8_t pixelNum, uint8_t redVal, uint8_t greenVal, uint8_t blueVal);
void kitt(uint8_t red, uint8_t green, uint8_t blue, uint8_t eyeSize);
void show();
uint16_t read_adc(void);
void init_adc(void);
void getPixelColor(uint8_t pixelNum, uint8_t *redVal, uint8_t *greenVal, uint8_t *blueVal);

//struct with pixel variables
typedef struct{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t reddata;
	uint8_t greendata;
	uint8_t bluedata;
	}pixel;
	
//struct variable attached to array of pixels
static volatile pixel allpixels[numPIXELS];

//void that updates strip data
void sendData(){
	for (uint8_t i = 0; i < numPIXELS; i++)
	{
	allpixels[i].reddata = (uint8_t) allpixels[i].red;
	allpixels[i].greendata = (uint8_t) allpixels[i].green;
	allpixels[i].bluedata = (uint8_t) allpixels[i].blue;
	}
}

//another drawing function, but with struct data
void drawStrip(){
		for (uint8_t i = 0; i < numPIXELS; i++){
			sendPixel (allpixels[i].reddata, allpixels[i].greendata, allpixels[i].bluedata);
		}
		while(!(TCE0.INTFLAGS & TC0_OVFIF_bm));
		TCE0.INTFLAGS |= TC0_OVFIF_bm;
		TCE0.CCABUF = 0;
		show();
}

// Sends one byte to the LED strip by SPI.
void sendByte (uint8_t b){
	unsigned char bit;
	uint8_t mask = MASK;
	//check all 8 bits
	for (bit = 0; bit < 8; bit++){
		while(!(TCE0.INTFLAGS & TC0_OVFIF_bm));
		TCE0.INTFLAGS |= TC0_OVFIF_bm;
		if (b & mask){ 				// is high-order bit set? (bitwise AND 128 (half the string))
		TCE0.CCABUF = 40;			// 40 pulses long
		} else {
		TCE0.CCABUF = 10;			// 10 pulses long
		}		
		mask >>= 1;					// shift next bit into high-order position
	}
}

// Send a single pixel worth of information.  Turn interrupts off while using.
void sendPixel (uint8_t r, uint8_t g, uint8_t b){
	sendByte (g);        // NeoPixel wants colors in green-then-red-then-blue order (datasheet)
	sendByte (r);
	sendByte (b);
}

// Display a single color on the whole string.  Turn interrupts off while using.
void showColor (unsigned int count, unsigned char r , unsigned char g , unsigned char b){
	unsigned int pixel;
	for (pixel = 0; pixel < count; pixel++){
		sendPixel (r, g, b);
	}
	while(!(TCE0.INTFLAGS & TC0_OVFIF_bm));
	TCE0.INTFLAGS |= TC0_OVFIF_bm;
	TCE0.CCABUF = 0;
	show();
}

void ws2812_init(){
		/* INIT PINS */
		PORTE.DIRSET = PIN0_bm;		//output pin for strip data
		/* INIT PINS */
		
		/* INIT STRIP PWM */
		TCE0.CTRLB	= TC_WGMODE_SINGLESLOPE_gc | TC0_CCAEN_bm;
		TCE0.PER	= 50;
		TCE0.CCA	= 0;
		TCE0.CTRLA	= TC_CLKSEL_DIV1_gc;
	
	show();
	sendPixel (0, 0, 0);	//all leds off
	show();
}

// latch the colors (wait long enough without sending any bits to allow the pixels to latch and
// display the last sent frame)
void show(){
	_delay_us(2340);            // >=50us
}

//target specific pixel(s)
void setPixel(uint8_t pixelNum, uint8_t redVal, uint8_t greenVal, uint8_t blueVal){
	allpixels[pixelNum].red = redVal;
	allpixels[pixelNum].green = greenVal;
	allpixels[pixelNum].blue = blueVal;
}

void getPixelColor(uint8_t pixelNum, uint8_t *redVal, uint8_t *greenVal, uint8_t *blueVal){
		*redVal = allpixels[pixelNum].red;
		*greenVal = allpixels[pixelNum].green;
		*blueVal = allpixels[pixelNum].blue;
}

//create getPixelColor() function

//ANIMATIONS//
//target all pixels at once
void setAll(uint8_t red, uint8_t green, uint8_t blue){
	for (uint8_t i = 0; i < numPIXELS; i++)
	{
		allpixels[i].red = red;
		allpixels[i].green = green;
		allpixels[i].blue = blue;
		sendData();
	}
}

void breatheAnimation(){
	//statically allocated memory, not automatic allocated memory, it remains in memory while the function is ended
	//works as a non-static global too. the static makes sure index isn't reset while code is running
	static uint8_t index = 0;
	for (uint8_t i = 0; i < numPIXELS; i++)
	{
	allpixels[i].blue = lookup[index];	
	_delay_us(1000);
	}
	//index increases 40 times (2x20 fade on and off) and modulo function makes it 0 again and restarts routine
	index = (index + 1) % 40;
	sendData();
}

//1
void sequenceAnimation(){
	//static uint8_t count = 0;
	static uint8_t previus = 0;
	for (uint8_t j = 0; j < 5; j++)
	{
		for (uint8_t i = 0; i < numPIXELS; i++)
		{
			setPixel(i /*count*/, 0, 0, 10);
			sendData();
			drawStrip();
			_delay_ms(5);
			previus = i;
			setPixel(previus, 0, 0, 0);
		}
		//previus = count % numPIXELS;
		//count = (count + 1) % numPIXELS;
		//previus = i;
		//setPixel(previus, 0, 0, 0);
	}
	animation = 5;
}

//2
void kitt(uint8_t red, uint8_t green, uint8_t blue, uint8_t eyeSize){
	for (uint8_t k = 0; k < 5; k++)
		{
		//one way
		for (uint8_t i = 0; i < numPIXELS-eyeSize-2; i++)
		{
			setAll(0,0,0);
			setPixel(i, red/10, green/10, blue/10);
			for (uint8_t j = 1; j <= eyeSize; j++)
			{
				setPixel(i+j, red, green, blue);
			}
			setPixel(i+eyeSize+1, red/10, green/10, blue/10);
			sendData();
			_delay_ms(2);
			drawStrip(); //might not work, sends data more than 800kHz
		}
		_delay_ms(30);
	
		//other way
		for (uint8_t i = numPIXELS; i > 0; i--)
		{
			setAll(0,0,0);
			setPixel(i, red/10, green/10, blue/10);
			for (uint8_t j = 1; j <= eyeSize; j++)
			{
				setPixel(i+j, red, green, blue);
			}
			setPixel(i+eyeSize+1, red/10, green/10, blue/10);
			sendData();
			_delay_ms(2);
			drawStrip(); //might not work, sends data more than 800kHz
		}
		_delay_ms(30);
	}
	animation = 1;
}

/* //kitt without for loops//
void kitt(uint8_t red, uint8_t green, uint8_t blue, uint8_t eyeSize){

	static uint8_t i = 0;
	
	setAll(0,0,0);
	setPixel(i, red/10, green/10, blue/10);
	for (uint8_t j = 1; j <= eyeSize; j++)
	{
		setPixel(i+j, red, green, blue);
	}
	setPixel(i+eyeSize+1, red/10, green/10, blue/10);
	sendData();
	_delay_ms(5);	//speeddelay
	i = (i + 1) % (numPIXELS-eyeSize-2);
}
*/

//5
void strobe(){
	for (uint8_t i = 0; i < 15; i++)
	{
		//three fast white flashes
		for (uint8_t j = 0; j < 3; j++)
		{
			setAll(10, 10, 10);
			sendData();
			drawStrip();
			_delay_ms(3);
			setAll(0,0,0);
			drawStrip();
			_delay_ms(20);
		}
		_delay_ms(300);
	}
	animation = 6;
}

//6
void halfhalf(){
	for (uint8_t k = 0; k < 10; k++)
	{
		for (uint8_t i = 0; i < (numPIXELS/2); i++)
		{
			setPixel(i, 15, 0, 0);
		}
		sendData();
		drawStrip();
		_delay_ms(600);
		setAll(0,0,0);
		for (uint8_t i = numPIXELS; i >= (numPIXELS/2); i--)
		{
			setPixel(i, 15, 0, 0);
		}
		sendData();
		drawStrip();
		_delay_ms(600);
		setAll(0,0,0);
	}
	animation = 2;
}

//7
//add function to sense volume and adjust sensitivity by that volume
void micEffect(uint8_t red, uint8_t green, uint8_t blue){
	uint16_t res = 0;
	double vinp;
	double lightVal = 0;
	uint16_t sensitivity = 180;		//lower value means higher sensitivity, everything below 180 is noise
	uint8_t intensity = 4;			//higher value means less light
	
	res = read_adc();
	vinp  = ((double) res * VREF / (MAX_VALUE + 1) - VOFFSET);
	lightVal = (310 * vinp);		// (255 / 0.832) * vinp = correct light signal for that mic signal
	if (lightVal > sensitivity)
	{
		setAll(lightVal/intensity, lightVal/intensity, lightVal/intensity);
		//make function that dims light of set leds
		drawStrip();
	} else {
		setAll(0,0,0);
	}
}

//average handling slows down function, try making it without averageing, but with dampening of mic signal
//8
void vu(){
	uint16_t res = 0;
	uint16_t noice = 600;		//lower value means higher sensitivity, everything below 180 is noise
	uint8_t height = 0;
	uint8_t average;
	uint8_t minLvl = 0;
	uint8_t maxLvl = 60;
	double maxVinp = 0.832;		//double value had better decimal points, 15 decimal digits total
	double vinp;
	static uint8_t count = 0;
	static int8_t total = 0;
	
	
	res = read_adc();
	//res = (res <= noice) ? 0 : (res - noice);
	if (res > 0)	//only use positive adc readings
	{
		vinp  = ((double) res * VREF / (MAX_VALUE + 1) - VOFFSET);
		height = (numPIXELS / (maxVinp - minLvl)) * (vinp - maxLvl);	//calculate height reach of pixels as a function of mic signal
		total += height;
	
		if (count == 1)
		{
			average = total / count;
			count = (count % 1);	//reset count after 5 cycles
			if (res > noice)
			{
				setAll(0,0,0);	//reset strip to dark
				drawStrip();
				//try removing noice from signal before sending pixeldata
				for (uint8_t i = 0; i <= average - 29; i++)
				{
					setPixel(i, 100, 0, 0);
					sendData();
				}
				drawStrip();
				_delay_ms(2);
				total = 0;
			}
		}
		count++;
	}
}

//ANIMATIONS//

void init_adc(void)
{
	PORTA.DIRCLR     = PIN2_bm;                          // configure PA2 as input for ADCA
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;            // PA2 to channel 0
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;  // channel 0 single-ended
	ADCA.REFCTRL     = ADC_REFSEL_INTVCC_gc;             // internal VCC/1.6 reference
	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc;          // 12 bits conversion, unsigned, no freerun
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;           // 2MHz/16 = 125kHz
	ADCA.CTRLA       = ADC_ENABLE_bm;                    // enable adc
}

uint16_t read_adc(void)
{
	uint16_t res;

	ADCA.CH0.CTRL |= ADC_CH_START_bm;                    // start ADC conversion
	while ( !(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm) ) ;    // wait until it's ready
	res = ADCA.CH0.RES;
	ADCA.CH0.INTFLAGS |= ADC_CH_CHIF_bm;                 // reset interrupt flag

	return res;                                          // return measured value
}


int main(void)
{
	Config32MHzClock();
	
	ws2812_init();
	
	init_adc();
	
	//timer that executes interrupt
	TCF0.CTRLB		= TC_WGMODE_NORMAL_gc;
	TCF0.INTCTRLA	= TC_OVFINTLVL_LO_gc;
	TCF0.PER		= 39;		//PER register sets how far the timer will count	32.000.000/1*(39+1) = 800.000Hz
	TCF0.CTRLA		= TC_CLKSEL_DIV1_gc;
	
	PMIC.CTRL		|= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
	
	animation = ANIMATION;
	
	while (1)
	{
		asm volatile ("nop");
	}
}

ISR(TCF0_OVF_vect)
{
	switch(animation)
	{
		case 0:
		breatheAnimation();
		break;
		case 1:
		sequenceAnimation();
		break;
		case 2:
		kitt(10, 0, 0, 5);	//red, green, blue, eyesize
		break;
		case 3:
		setAll(10, 0, 0);
		break;
		case 4:
		//set part of strip color
		setPixel(20, 10, 0, 0);
		setPixel(22, 0, 10, 10);
		setPixel(30, 0, 10, 0);
		sendData();
		break;
		case 5:
		strobe();
		break;
		case 6:
		halfhalf();
		break;
		case 7:
		micEffect(10, 10, 10);
		break;
		case 8:
		vu();
		break;
		default:
		asm volatile ("nop");
		break;
	}
	drawStrip();
}
