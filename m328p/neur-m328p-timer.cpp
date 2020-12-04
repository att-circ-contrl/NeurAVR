// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega328P - Timer functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"


//
// Notes

// The ATmega328 has three timers: Timer 0 (8-bit), Timer 1 (16-bit),
// and Timer 2 (8-bit).
// We're using Timer 1 for the RTC.



//
// Functions


// RTC intialization routine.
// Unhooks all timers and initializes the RTC timer.
// An RTC rate of 0 disables the RTC.

void Timer_Init(uint32_t mcu_hz, uint32_t rtc_hz)
{
  uint32_t clocks_per_tick;
  uint32_t scratch;

  // First, disable all timer interrupts.
  TIMSK0 = 0;
  TIMSK1 = 0;
  TIMSK2 = 0;

  // We can now alter timer settings without further locking.

  // Set all timers to inactive, CTC mode.

  TCCR0A = 0x02;
  TCCR0B = 0x00;

  TCCR1A = 0x00;
  TCCR1B = 0b01000;
  TCCR1C = 0x00; // Probably not needed, but writing 0 is still ok.

  TCCR2A = 0x02;
  TCCR2B = 0x00;

  // Initialize the timestamp and reset the callback.
  rtc_timestamp = 0;
  rtc_usercallback = NULL;


  // Initialize our timer if we've been given a nonzero rate.
  if (0 < rtc_hz)
  {
    // Figure out what Timer 1's ceiling should be.
    // NOTE - We're forcing a /1 divisor, which constrains range.
    clocks_per_tick = mcu_hz / rtc_hz;
    // For a /1 prescaler, f = cpuclk / (1 + OCRnA).
    // So, OCRnA = (cpuclk / f) - 1.
    // Do boundary checking just to be safe.
    if (0 < clocks_per_tick)
      clocks_per_tick--;
    if (0xffff < clocks_per_tick)
      clocks_per_tick = 0xffff;

    // Configure Timer 1.
    // NOTE - For 16-bit registers, write high first, read low first.
    // NOTE - Type coercion has to be done carefully here.
    // Otherwise we can get truncation before we want it.

    scratch = (clocks_per_tick >> 8) & 0xff;
    OCR1AH = (uint8_t) scratch;
    scratch = clocks_per_tick & 0xff;
    OCR1AL = (uint8_t) scratch;

    // Reset the counter value, to be safe.
    TCNT1H = 0x00;
    TCNT1L = 0x00;

    // Enable the timer with a /1 divisor.
    TCCR1B = 0b01001;
    TIMSK1 = 0x02;
  }
}



// RTC Interrupt service routine.
// This updates the RTC timestamp, and optionally calls a user-provided
// function.

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
  // This may overflow for very fast or very long running clocks.
  // That's tolerable.
  rtc_timestamp++;

  // This really, really has to return quickly.
  // Not just within one RTC tick - it has to return before _any_ other
  // interrupt-driven event would happen _twice_.
  if (NULL != rtc_usercallback)
    (*rtc_usercallback)();
}


//
// This is the end of the file.
