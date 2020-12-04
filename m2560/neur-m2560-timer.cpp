// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega2560 - Timer functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"


//
// Notes

// The ATmega2560 has six timers: Timers 0 and 2 (8-bit) and Timers 1, 3, 4,
// and 5 (16-bit). Timers 0 and 2 are not identical.
// We're using Timer 5 for the RTC.



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
  TIMSK3 = 0;
  TIMSK4 = 0;
  TIMSK5 = 0;

  // We can now alter timer settings without further locking.


  // Set all timers to inactive, CTC mode, no output pins.

  // 8-bit timers.

  TCCR0A = 0b0010;
  TCCR0B = 0;

  TCCR2A = 0b0010;
  TCCR2B = 0;


  // 16-bit timers.
  // TCCRnC is for forcing compare matches for synchronization; ignore it.

  TCCR1A = 0;
  TCCR1B = 0b01000;

  TCCR3A = 0;
  TCCR3B = 0b01000;

  TCCR4A = 0;
  TCCR4B = 0b01000;

  TCCR5A = 0;
  TCCR5B = 0b01000;


  // Initialize the timestamp and reset the callback.

  rtc_timestamp = 0;
  rtc_usercallback = NULL;


  // Initialize our timer if we've been given a nonzero rate.

  // We're using timer 5.
  // The only interrupt we care about is output compare A.
  // NOTE - We're forcing a /1 divisor, which constrains range.

  if (0 < rtc_hz)
  {
    // This will get rounded, but it's as close as we're going to get.
    clocks_per_tick = mcu_hz / rtc_hz;

    // Convert this to a compare-match value.
    // For a /1 prescaler, f = cpuclk / (1 + OCRnA).
    // So, OCRnA = (cpuclk / f) - 1.
    // Do boundary checking just to be safe, so we don't overflow or
    // underflow while doing this calculation.
    if (0 < clocks_per_tick)
      clocks_per_tick--;
    if (0xffff < clocks_per_tick)
      clocks_per_tick = 0xffff;

    // Configure Timer 5.
    // NOTE - For 16-bit registers, write high first, read low first.
    // NOTE - Type coercion has to be done carefully here.
    // Otherwise we can get truncation before we want it.

    scratch = (clocks_per_tick >> 8) & 0xff;
    OCR5AH = (uint8_t) scratch;
    scratch = clocks_per_tick & 0xff;
    OCR5AL = (uint8_t) scratch;

    // We don't care about OCRnB or OCRnC.

    // Reset the counter value, to be safe.
    TCNT5H = 0x00;
    TCNT5L = 0x00;

    // Enable the timer with a /1 divisor.
    TCCR5B = 0b01001;
    TIMSK5 = (1 << OCIE5A);
  }
}



// RTC Interrupt service routine.
// This updates the RTC timestamp, and optionally calls a user-provided
// function.

ISR(TIMER5_COMPA_vect, ISR_BLOCK)
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
