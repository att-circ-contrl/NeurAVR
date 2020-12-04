// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega2560 - UART functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"



//
// Notes

// The ATmega2560 has 4 UARTs; UART 0 (normal) or 1 (alternate) is our primary.



//
// Macros

#define UART_CSRA_2X 0x02
#define UART_CSRB_OFF 0x00
// The RX complete interrupt is always on. TX complete is always off.
// When transmitting, the Data Register Empty interrupt is on.
#define UART_CSRB_TXIDLE 0b10011000
#define UART_CSRB_TXON   0b10111000
// We want asynchronous mode, 8n1.
#define UART_CSRC 0b00000110

// Wrappers for UART control and data registers.

#if UART_USE_ALTERNATE

// Alternate: UART 1.
#define UREG_CSRA UCSR1A
#define UREG_CSRB UCSR1B
#define UREG_CSRC UCSR1C
#define UREG_BRRH UBRR1H
#define UREG_BRRL UBRR1L
#define UREG_DR   UDR1
#define UVECT_RX    USART1_RX_vect
#define UVECT_UDRE  USART1_UDRE_vect

#else

// Normal: UART 0.
#define UREG_CSRA UCSR0A
#define UREG_CSRB UCSR0B
#define UREG_CSRC UCSR0C
#define UREG_BRRH UBRR0H
#define UREG_BRRL UBRR0L
#define UREG_DR   UDR0
#define UVECT_RX    USART0_RX_vect
#define UVECT_UDRE  USART0_UDRE_vect

#endif



//
// Variables

// Actual baud rate set.
uint32_t real_baud_rate = 0;



//
// Functions


// Configures the primary UART for the specified baud rate.
// A baud rate of 0 turns it off.
// Which UART is primary is architecture-dependent.

void UART_Init(uint32_t mcu_hz, uint32_t baud_rate)
{
  uint32_t ubrr_value;
  uint32_t scratch;

  if (0 == baud_rate)
  {
    real_baud_rate = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      // Turn off interrupts _and_ rx/tx.
      UREG_CSRA = UART_CSRA_2X;
      UREG_CSRB = UART_CSRB_OFF;
      UREG_CSRC = UART_CSRC;

      // Ignore the baud rate registers and data register.
    }
  }
  else
  {
    // Calculate the desired baud setting before locking.
    ubrr_value = mcu_hz / (baud_rate << 3);
    if (0 < ubrr_value)
      ubrr_value--;
    // Clamp this to 12 bits.
    if (0x0fff < ubrr_value)
      ubrr_value = 0x0fff;

    // Translate this into a record of the real baud rate.
    real_baud_rate = mcu_hz / ((ubrr_value + 1) << 3);

    // Check the next higher value; floor(ubrr) is not always ideal.
    scratch = mcu_hz / ((ubrr_value + 2) << 3);

    // The smaller ubrr will overshoot the baud rate, the larger will
    // undershoot.
    if ( (ubrr_value < 0x0fff)
      && ( (real_baud_rate - baud_rate) > (baud_rate - scratch) ) )
    {
      // Rounding up gave a better approximation.
      ubrr_value++;
      real_baud_rate = scratch;
    }

    // Disable the UART, set the baud rate, and renable the UART.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      // Turn off the UART.
      UREG_CSRB = UART_CSRB_OFF;

      // Store UBRR.
      // NOTE - Write high first, then low.
      // NOTE - Type coercion has to be done carefully here.
      // Otherwise we can get truncation before we want it.

      scratch = (ubrr_value >> 8) & 0xff;
      UREG_BRRH = (uint8_t) scratch;
      scratch = ubrr_value & 0xff;
      UREG_BRRL = (uint8_t) scratch;

      // Reinitialize buffers.
      UART_InitBuffers_ISR();

      // Turn on the UART in TX-idle mode.
      UREG_CSRB = UART_CSRB_TXIDLE;
    }
  }
}



// Returns the actual baud rate set, or 0 if the UART is off.

uint32_t UART_QueryBaud(void)
{
  return real_baud_rate;
}



// This is a transmission-start hook called after a string is queued.
// It re-enables need-character interrupts if they aren't aready enabled.
// The caller is responsible for any needed locking.

void UART_EnableTransmit_ISR(void)
{
  UREG_CSRB = UART_CSRB_TXON;
}



// Interrupt service routine for "RX complete".

ISR(UVECT_RX, ISR_BLOCK)
{
  uint8_t thischar;

  thischar = UREG_DR;

  UART_HandleRecvChar_ISR(thischar);
}



// Interrupt service routine for "Data register ready".
// We don't care whether TX has finished or not, just whether it wants
// more data.

ISR(UVECT_UDRE, ISR_BLOCK)
{
  char thischar;

  thischar = 0;

  if (UART_GetNextSendChar_ISR(thischar))
  {
    // We have a character; transmit it.
    UREG_DR = thischar;
  }
  else
  {
    // Nothing more to send.
    // Keep TX enabled but disable the UDRE interrupt.
    UREG_CSRB = UART_CSRB_TXIDLE;
  }
}



//
// This is the end of the file.
