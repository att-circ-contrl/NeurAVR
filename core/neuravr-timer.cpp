// Attention Circuits Control Laboratory - Atmel AVR firmware
// Common core - Timer functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"



//
// Variables


// Real-time clock variables.

volatile uint32_t rtc_timestamp;
void (*rtc_usercallback)(void) = NULL;



//
// Functions


// Real-time clock functions.


// Clears the RTC timestamp.

void Timer_Reset(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    rtc_timestamp = 0;
  }
}



// Reads the real-time clock timestamp.

uint32_t Timer_Query(void)
{
  uint32_t result;

#ifdef NEUREMU
  // Suppress warning.
  result = 0;
#endif

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    result = rtc_timestamp;
  }

  return result;
}



// Reads the real-time clock timestamp while within an ISR or other
// locked code. This avoids an ATOMIC_BLOCK call.

uint32_t Timer_Query_ISR(void)
{
  return rtc_timestamp;
}



// Specifies a user-defined function to call during timer interrupts.
// _All_ interrupts, not just timer interrupts, are disabled while this
// runs, so it has to return very quickly.

void Timer_RegisterCallback(void (*callback)(void))
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    rtc_usercallback = callback;
  }
}



//
// This is the end of the file.
