// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega2560 - Header.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Macros

// ADC-related macros.

#define ADC_CHANNEL_COUNT 8


// UART-related macros.

// Switch for UART0 (default) vs UART1 (alternate).
#define UART_USE_ALTERNATE 0

// Sizes should be powers of 2, so we can do modulo math by masking.
// The 2560 has 8k of SRAM. This uses 1k.
#define UART_LINE_COUNT 8
// Each line can have 2^bits characters.
#define UART_LINE_BITS 7
#define UART_LINE_SIZE (1 << UART_LINE_BITS)

// This switch identifies whether to use _near or _far pointers for
// flash-stored variables.
// Even for large flash memories, PROGMEM variables are placed in the lower
// 64k first, so near pointers usually work.
// FIXME - Forcing far, for robustness on the 2560.
#define USE_FAR_FLASH_POINTERS 1


//
// This is the end of the file.
