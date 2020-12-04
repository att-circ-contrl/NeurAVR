// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega328P - MCU initialization functions.
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

// The ATmega328 has one UART: UART 0. That's our primary.



//
// Functions


// MCU initialization routine.
// Initializes the MCU to a known-good state.

void MCU_Init(void)
{
  // Make very sure interrupts are off during initialization, and turned
  // on afterwards.
  // This is more robust than cli() alone.
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    // Initialize I/O pins.
    // Pull-ups enabled globally, all pins high-Z inputs locally.

    // Clear Pull-Up Disable, enabling pull-ups.
    // We can ignore IVSEL and IVCE; they only change when a specific
    // song and dance are performed.
    MCUCR = 0x00;

    // Set everything to high-Z input.

    DDRB = 0x00;
    DDRC = 0x00;
    DDRD = 0x00;

    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;


    // Initialize peripherals.

    // FIXME - Relying on mcu_hz not mattering if we're disabling a device.
    // This may change!

    // Disable all timers.
    Timer_Init(0, 0);

    // Disable the UART.
    UART_Init(0, 0);

    // Disable the ADC.
    // FIXME - ADC NYI.
  }

  // ATOMIC_FORCEON means interrupts are enabled by this point.
}


//
// This is the end of the file.
