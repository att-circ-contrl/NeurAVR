// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega2560 - MCU initialization functions.
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

// The ATmega2560 has 4 UARTs; UART 0 (normal) or 1 (alternate) is our primary.



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
    // There is no port I.

    DDRA = 0x00;
    DDRB = 0x00;
    DDRC = 0x00;
    DDRD = 0x00;
    DDRE = 0x00;
    DDRF = 0x00;
    DDRG = 0x00;
    DDRH = 0x00;
    DDRJ = 0x00;
    DDRK = 0x00;
    DDRL = 0x00;

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    PORTF = 0x00;
    PORTG = 0x00;
    PORTH = 0x00;
    PORTJ = 0x00;
    PORTK = 0x00;
    PORTL = 0x00;


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
