// Attention Circuits Control Laboratory - Library Tests
// Blinking light demo.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

//
// Includes

#include "neuravr.h"



//
// Macros

#define CPU_SPEED 16000000ul
#define RTC_TICKS_PER_SECOND 1000ul

// LED pin varies with architecture.

#ifdef __AVR_ATmega328P__

// D13 is B5 on the Uno.
#define LED_ON 0b00100000
#define LED_OFF 0
#define LED_PORT PORTB
#define LED_DIR DDRB

#endif

#ifdef __AVR_ATmega2560__

// D13 is B7 on the Mega 2560.
#define LED_ON 0b10000000
#define LED_OFF 0
#define LED_PORT PORTB
#define LED_DIR DDRB

#endif



//
// Functions

void TimerCallback(void)
{
  uint32_t thistime;

  thistime = Timer_Query_ISR();

  if (thistime & 256)
  { LED_PORT = LED_OFF; }
  else
  { LED_PORT = LED_ON; }
}



//
// Main Program

int main(void)
{
  uint32_t thistime;

  MCU_Init();

  LED_DIR = LED_ON;
  LED_PORT = LED_OFF;

  Timer_Init(CPU_SPEED, RTC_TICKS_PER_SECOND);

  do
  {
    thistime = Timer_Query();

    if (thistime & 512)
    { LED_PORT = LED_OFF; }
    else
    { LED_PORT = LED_ON; }
  }
  while (5000 > thistime);

  Timer_RegisterCallback(&TimerCallback);

  while (1) ;

  return 0;
}


//
// This is the end of the file.
