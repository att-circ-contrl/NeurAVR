// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega328P - Header.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Macros

// ADC-related macros.

#define ADC_CHANNEL_COUNT 6


// UART-related macros.

// Sizes should be powers of 2, so we can do modulo math by masking.
// The 328P has 2k of SRAM. This uses 0.5k.
#define UART_LINE_COUNT 8
// Each line can have 2^bits characters.
#define UART_LINE_BITS 6
#define UART_LINE_SIZE (1 << UART_LINE_BITS)

// This switch identifies whether to use _near or _far pointers for
// flash-stored variables.
// Even for large flash memories, PROGMEM variables are placed in the lower
// 64k first, so near pointers usually work.
#define USE_FAR_FLASH_POINTERS 0


//
// This is the end of the file.
