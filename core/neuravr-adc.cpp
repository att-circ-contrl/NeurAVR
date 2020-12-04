// Attention Circuits Control Laboratory - Atmel AVR firmware
// Common core - ADC functions.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"

// FIXME - We're using snprintf().
#include <stdio.h>



//
// Variables


// ADC variables.

// Converted data and queue.
bool ADC_idle = true;
bool ADC_needs_conversion[ADC_CHANNEL_COUNT];
bool ADC_data_ready[ADC_CHANNEL_COUNT];
uint16_t ADC_data[ADC_CHANNEL_COUNT];



//
// Functions


// Private ADC functions.


// This reinitializes the ADC buffer and conversion flags.

void ADC_ReInitBuffer(void)
{
  int cidx;

  for (cidx = 0; cidx < ADC_CHANNEL_COUNT; cidx++)
  {
    ADC_idle = true;
    ADC_needs_conversion[cidx] = false;
    ADC_data_ready[cidx] = false;
    ADC_data[cidx] = 0;
  }
}


// Public ADC functions.


// Performs housekeeping polling for the ADC.
// This is typically called from the timer interrupt.
// At 16 MHz, polling at 8 kHz is optimal (conversions take 1700 clocks).

void ADC_HousekeepingPoll(void)
{
  uint8_t cidx, next_channel;
  uint8_t count;

  if ( (!ADC_idle) && (!ADC_IsADCBusy()) )
  {
    // A conversion just finished.

    count = 0;
    next_channel = 0;

    for (cidx = 0; cidx < ADC_CHANNEL_COUNT; cidx++)
    {
      if (ADC_needs_conversion[cidx])
      {
        count++;

        if (1 == count)
        {
          ADC_needs_conversion[cidx] = false;
          ADC_data_ready[cidx] = true;
          ADC_data[cidx] = ADC_GetConversionResult();
        }
        else if (2 == count)
        {
          next_channel = cidx;
        }
      }
    }

    // See if we've converted the last sample.
    // Queue the next sample if not.
    if (1 >= count)
      ADC_idle = true;
    else
      ADC_ReadFromChannel(next_channel);
  }
}



// Queues conversion of analog signals on the specified ADC channels.
// This is ignored if a previous conversion is still in progress.
// Any unread pending data is discarded.

void ADC_StartConversion(uint8_t channel_mask)
{
  uint8_t cidx;
  bool need_first;
  uint8_t first_channel;

  if (ADC_idle)
  {
    ADC_ReInitBuffer();

    need_first = true;
    first_channel = 0;

    for (cidx = 0; cidx < ADC_CHANNEL_COUNT; cidx++)
    {
      if (channel_mask & (1 << cidx))
      {
        ADC_needs_conversion[cidx] = true;

        if (need_first)
        {
          first_channel = cidx;
          need_first = false;
        }
      }
    }

    if (!need_first)
    {
      ADC_idle = false;
      ADC_ReadFromChannel(first_channel);
    }
  }
}



// Returns true if the queued conversion has completed and has unread data.

bool ADC_IsDataReady(void)
{
  bool result;
  uint8_t cidx;

  result = false;

  if (ADC_idle)
  {
    for (cidx = 0; cidx < ADC_CHANNEL_COUNT; cidx++)
      if (ADC_data_ready[cidx])
        result = true;
  }

  return result;
}



// Blocks until the queued conversion (if any) completes.
// Granularity is several hundred clock cycles due to busy-wait padding.
// Interrupts are still handled during this time.

void ADC_WaitForData(void)
{
  while (!ADC_IsDataReady())
  {
    // Busy-wait so as to not hammer ATOMIC_BLOCK().
    // This waits about 600 clock cycles (40 us at 16 MHz).
    _delay_loop_1(200);
  }
}



// Reads data for the next completed but unread analog sample.
// Returns true if unread data was present and false otherwise.
// Channel ID is set to 0..7; this channel's mask is 1 << channel_id.

bool ADC_ReadPendingSample(uint16_t &data, uint8_t &channel_id)
{
  bool found;
  uint8_t cidx;

  found = false;

  if (ADC_idle)
  {
    for (cidx = 0; (!found) && (cidx < ADC_CHANNEL_COUNT); cidx++)
      if (ADC_data_ready[cidx])
      {
        ADC_data_ready[cidx] = false;
        found = true;
        data = ADC_data[cidx];
        channel_id = cidx;
      }
  }

  return found;
}



//
// This is the end of the file.
