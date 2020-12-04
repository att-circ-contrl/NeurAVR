// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega328P - Digital GPIO functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"



//
// Notes

// See "NOTES-pins" for pin mappings.

// The 8-bit digital bank uses D5..D7 for GP0..DP2 and B0..B4 for GP3..GP7.
// The 16-bit bank is not mapped.



//
// Macros


// Pin use masks for ports that are only partly mapped to I/Os.
// We want unmapped bits to stay high-Z inputs, period.

#define GPMASK_PORTD 0xe0
#define GPMASK_PORTB 0x1f



//
// Private Global Variables


// Direction masks for the various ports.
// Default to "all input", per MCU initialization.

uint8_t dirmask_portd = 0x00;
uint8_t dirmask_portb = 0x00;

// Last values written to the various ports.
// This is a combination of data for outputs and pullup state for inputs.
uint8_t data_d = 0x00;
uint8_t data_b = 0x00;

// Last values written by the user.
uint8_t lastval_8 = 0x00;



//
// Functions


// 8-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO8_SelectOutputs(uint8_t output_mask)
{
  dirmask_portd = (output_mask & 0x07) << 5;
  dirmask_portb = (output_mask & 0xf8) >> 3;

  dirmask_portd &= GPMASK_PORTD;
  dirmask_portb &= GPMASK_PORTB;

  DDRD = dirmask_portd;
  DDRB = dirmask_portb;
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO8_WriteData(uint8_t output_data)
{
  uint8_t scratch_d, scratch_b;

  lastval_8 = output_data;

  scratch_d = (output_data & 0x07) << 5;
  scratch_b = (output_data & 0xf8) >> 3;

  // Keep bits that are outputs.

  scratch_d &= dirmask_portd;
  scratch_b &= dirmask_portb;

  // Combine this with pull-up state.

  data_d &= ~dirmask_portd;
  data_d |= scratch_d;

  data_b &= ~dirmask_portb;
  data_b |= scratch_b;


  PORTD = data_d;
  PORTB = data_b;
}


// Returns the last written value. This lets the user set/clear bits
// without disturbing bits that are to remain the same.

uint8_t IO8_GetOutputValue(void)
{
  return lastval_8;
}


// Enables pull-ups on selected GPIO lines. Only configured inputs have
// pull-ups. 1 = pull-up, 0 = floating.

void IO8_SetPullups(uint8_t pullup_mask)
{
  uint8_t scratch_d, scratch_b;

  scratch_d = (pullup_mask & 0x07) << 5;
  scratch_b = (pullup_mask & 0xf8) >> 3;

  // Keep bits that are _not_ outputs, but that are still mapped to GPIOs.

  scratch_d &= ~dirmask_portd;
  scratch_d &= GPMASK_PORTD;

  scratch_b &= ~dirmask_portb;
  scratch_b &= GPMASK_PORTB;

  // Combine this with output state.

  data_d &= dirmask_portd;
  data_d |= scratch_d;

  data_b &= dirmask_portb;
  data_b |= scratch_b;


  PORTD = data_d;
  PORTB = data_b;
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint8_t IO8_ReadData(void)
{
  uint8_t scratch_d, scratch_b;

  scratch_d = PIND;
  scratch_b = PINB;

  // Keep bits that are _not_ outputs, but that are still mapped to GPIOs.

  scratch_d &= ~dirmask_portd;
  scratch_d &= GPMASK_PORTD;

  scratch_b &= ~dirmask_portb;
  scratch_b &= GPMASK_PORTB;

  // Map port bits to data bits.

  scratch_d >>= 5;
  scratch_d &= 0x07;

  scratch_b <<= 3;
  scratch_b &= 0xf8;

  return scratch_d | scratch_b;
}



// 16-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO16_SelectOutputs(uint16_t output_mask)
{
  // Not mapped; nothing to do.
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO16_WriteData(uint16_t output_data)
{
  // Not mapped; nothing to do.
}


// Returns the last written value. This lets the user set/clear bits
// without disturbing bits that are to remain the same.

uint16_t IO16_GetOutputValue(void)
{
  // Not mapped; nothing to do.
  return 0;
}


// Enables pull-ups on selected GPIO lines. Only configured inputs have
// pull-ups. 1 = pull-up, 0 = floating.

void IO16_SetPullups(uint16_t pullup_mask)
{
  // Not mapped; nothing to do.
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint16_t IO16_ReadData(void)
{
  // Not mapped; nothing to do.

  return 0;
}



//
// This is the end of the file.
