// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega2560 - Digital GPIO functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"



//
// Notes

// See "NOTES-pins" for pin mappings.

// The 8-bit digital bank uses H3..H6 for GP0..GP3, and B4..B7 for GP4..GP7.
// The 16-bit bank uses L0..L7 for GP8..GP15 and C0..C7 for GP16..GP23.



//
// Macros


// Pin use masks for ports that are only partly mapped to I/Os.
// We want unmapped bits to stay high-Z inputs, period.

#define GPMASK_PORTH (0x0f << 3)
#define GPMASK_PORTB 0xf0



//
// Private Global Variables


// Direction masks for the various ports.
// Default to "all input", per MCU initialization.

uint8_t dirmask_porth = 0x00;
uint8_t dirmask_portb = 0x00;
uint8_t dirmask_portl = 0x00;
uint8_t dirmask_portc = 0x00;

// Last values written to the various ports.
// This is a combination of data for outputs and pullup state for inputs.
uint8_t data_h = 0x00;
uint8_t data_b = 0x00;
uint8_t data_l = 0x00;
uint8_t data_c = 0x00;

// Last values written by the user.
uint8_t lastval_8 = 0x00;
uint16_t lastval_16 = 0x00;



//
// Functions


// 8-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO8_SelectOutputs(uint8_t output_mask)
{
  dirmask_porth = (output_mask & 0x0f) << 3;
  dirmask_portb = output_mask & 0xf0;

  dirmask_porth &= GPMASK_PORTH;
  dirmask_portb &= GPMASK_PORTB;

  DDRH = dirmask_porth;
  DDRB = dirmask_portb;
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO8_WriteData(uint8_t output_data)
{
  uint8_t scratch_h, scratch_b;

  lastval_8 = output_data;

  scratch_h = (output_data & 0x0f) << 3;
  scratch_b = output_data & 0xf0;

  // Keep bits that are outputs.

  scratch_h &= dirmask_porth;
  scratch_b &= dirmask_portb;

  // Combine this with pull-up state.

  data_h &= ~dirmask_porth;
  data_h |= scratch_h;

  data_b &= ~dirmask_portb;
  data_b |= scratch_b;


  PORTH = data_h;
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
  uint8_t scratch_h, scratch_b;

  scratch_h = (pullup_mask & 0x0f) << 3;
  scratch_b = pullup_mask & 0xf0;

  // Keep bits that are _not_ outputs, but that are still mapped to GPIOs.

  scratch_h &= ~dirmask_porth;
  scratch_h &= GPMASK_PORTH;

  scratch_b &= ~dirmask_portb;
  scratch_b &= GPMASK_PORTB;

  // Combine this with output state.

  data_h &= dirmask_porth;
  data_h |= scratch_h;

  data_b &= dirmask_portb;
  data_b |= scratch_b;


  PORTH = data_h;
  PORTB = data_b;
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint8_t IO8_ReadData(void)
{
  uint8_t scratch_h, scratch_b;

  scratch_h = PINH;
  scratch_b = PINB;

  // Keep bits that are _not_ outputs, but that are still mapped to GPIOs.

  scratch_h &= ~dirmask_porth;
  scratch_h &= GPMASK_PORTH;

  scratch_b &= ~dirmask_portb;
  scratch_b &= GPMASK_PORTB;

  // Map port bits to data bits.

  scratch_h >>= 3;
  scratch_h &= 0x0f;

  scratch_b &= 0xf0;

  return scratch_h | scratch_b;
}



// 16-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO16_SelectOutputs(uint16_t output_mask)
{
  uint16_t scratch_l, scratch_c;

  scratch_l = output_mask & 0xff;
  scratch_c = output_mask >> 8;

  dirmask_portl = (uint8_t) scratch_l;
  dirmask_portc = (uint8_t) scratch_c;

  DDRL = dirmask_portl;
  DDRC = dirmask_portc;
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO16_WriteData(uint16_t output_data)
{
  uint16_t word_l, word_c;
  uint8_t byte_l, byte_c;

  lastval_16 = output_data;

  word_l = output_data & 0xff;
  word_c = output_data >> 8;

  byte_l = (uint8_t) word_l;
  byte_c = (uint8_t) word_c;

  // Keep bits that are outputs.

  byte_l &= dirmask_portl;
  byte_c &= dirmask_portc;

  // Combine this with pull-up state.

  data_l &= ~dirmask_portl;
  data_l |= byte_l;

  data_c &= ~dirmask_portc;
  data_c |= byte_c;


  PORTL = data_l;
  PORTC = data_c;
}


// Returns the last written value. This lets the user set/clear bits
// without disturbing bits that are to remain the same.

uint16_t IO16_GetOutputValue(void)
{
  return lastval_16;
}


// Enables pull-ups on selected GPIO lines. Only configured inputs have
// pull-ups. 1 = pull-up, 0 = floating.

void IO16_SetPullups(uint16_t pullup_mask)
{
  uint16_t word_l, word_c;
  uint8_t byte_l, byte_c;

  word_l = pullup_mask & 0xff;
  word_c = pullup_mask >> 8;

  byte_l = (uint8_t) word_l;
  byte_c = (uint8_t) word_c;

  // Keep bits that are _not_ outputs.

  byte_l &= ~dirmask_portl;
  byte_c &= ~dirmask_portc;

  // Combine this with output state.

  data_l &= dirmask_portl;
  data_l |= byte_l;

  data_c &= dirmask_portc;
  data_c |= byte_c;


  PORTL = data_l;
  PORTC = data_c;
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint16_t IO16_ReadData(void)
{
  uint16_t word_l, word_c;
  uint8_t byte_l, byte_c;

  byte_l = PINL;
  byte_c = PINC;

  // Keep bits that are _not_ outputs.

  byte_l &= ~dirmask_portl;
  byte_c &= ~dirmask_portc;

  // Map port bits to data bits.

  word_l = byte_l;

  word_c = byte_c;
  word_c <<= 8;

  return word_l | word_c;
}



//
// This is the end of the file.
