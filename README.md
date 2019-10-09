# WS2812-AVR-Library
All code written by Thomas P.  
This library was programmed in Atmel Studio 7.0 in C language. An AVR library for the WS2812b ledstrip. This library is made for the AtXmega264A3U but can easily be modified for any other AVR microcontroller. It uses a microphone (with low pass filter) to produce light-effects for low frequencies in music.

Ledstrip security:  
1000uF 6,3V capacitor or higher across + and - terminals of ledstrip to prevent pixel damage. 300 to 500 Ohm resistor between microcontro. datapin and ledstrip data pin to prevent voltage spikes that damage first pixel.

Add the following existing files to your code. You dont need the rest of the files other than these:  
  avr_compiler.h  
  clock.c   for generating higher clock rate  
  stream.c  
  stream.h  
  uart.c  
  uart.h  
  usart_driver.c  
  usart_driver.h  
  WS2812.c  main file  
  
Datapin: D8 (PE0) on AtXmega256A3U  
Using timer (800kHz) and PWM to produce correct timing for executing PWM dutycycle.
