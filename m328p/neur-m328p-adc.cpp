// Attention Circuits Control Laboratory - Atmel AVR firmware
// ATmega328P - ADC functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"


//
// Notes

// The ATmega328P has 6 ADC inputs in 28-pin packages; 8 in 32-pin packages.
// The ATmega328P offers a 1.1V reference, which we're using.



//
// Macros

// Fixed portion of ADMUX.
// This selects the 1.1V reference and left-adjusted output.
// We still need a capacitor on the AREF pin; most boards have this.
#define ADMUX_BASE 0xe0

// For single-ended input, we OR in the channel number in bits 0..2.

// ADCSRB is set to a constant value.
// Comparator off, trigger to free-running.
// Trigger mode is actually ignored, as we're disabling auto-triggering.
#define ADCSRB_VALUE 0x00

// We're defining three states: ADC off, ADC ready, ADC starting.
// Remember that we need to write 1 to ADIF to clear it, not 0.
// That said, we can ignore ADIF and just watch ADSC.
// We're using a clock divisor of /128, for 1700 clocks per conversion.
// This should work acceptably with 8, 16, and 20 MHz system clocks.

// Write to ADIF to clear, no auto, no interrupts, prescaler zero.
#define ADCSRA_OFF 0x17

// Set the enable flag, keep auto-trigger off.
#define ADCSRA_READY (ADCSRA_OFF | 0x80)

// This sets the "start conversion" flag.
// The flag doubles as a "conversion in progress" flag.
#define ADCSRA_STARTFLAG 0x40
#define ADCSRA_START (ADCSRA_READY | ADCSRA_STARTFLAG)



//
// Functions


// Initializes the ADC, selecting unipolar input and the 1.1V reference.
// This is one-time hardware initialization.

void ADC_Init(void)
{
  // Disable digital inputs.
  DIDR0 = 0x00;

  // Force the ADC off.
  ADCSRA = ADCSRA_OFF;

  // Set remaining control registers to our desired values.
  ADCSRB = ADCSRB_VALUE;
  ADMUX = ADMUX_BASE; // Channel 0 selected.

  // Turn the ADC on.
  // NOTE - The first few ADC samples will be bogus, and the first sample
  // will take twice as long to arrive.
  ADCSRA = ADCSRA_READY;
}



// This checks ADC registers to see if a conversion is in progress.

bool ADC_IsADCBusy(void)
{
  bool result;
  uint8_t sra_value;

  result = false;

  // The "start conversion" flag stays high while the conversion is in
  // progress.
  sra_value = ADCSRA;
  if (sra_value & ADCSRA_STARTFLAG)
    result = true;

  return result;
}



// This starts a conversion on the specified channel.
// Channel ID is 0..5.

void ADC_ReadFromChannel(uint8_t channel_id)
{
  if (channel_id < ADC_CHANNEL_COUNT)
  {
    ADMUX = ADMUX_BASE | channel_id;
    ADCSRA = ADCSRA_START;
  }
}



// This returns the value of the last converted sample.
// This is scaled to use the full 16-bit range.

uint16_t ADC_GetConversionResult(void)
{
  uint16_t result_high, result_low;

  // Blithely assume we're reading this at an appropriate time.

  // Read LS first, MS last; the MS read resets the register.
  result_low = ADCL;
  result_high = ADCH;

  return (result_high << 8) | result_low;
}



//
// This is the end of the file.
