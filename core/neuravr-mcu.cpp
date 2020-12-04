// Attention Circuits Control Laboratory - Atmel AVR firmware
// Common core - MCU functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"



//
// Private Global Variables

// From the heap manager.
extern char *__brkval;



//
// Functions


// Debugging functions.

// This returns the distance between the top of the stack and the heap.
// FIXME - Implementation relies on fragile toolchain details!

uint16_t MCU_GetFreeMemory(void)
{
  long result;

#ifdef NEUREMU
  // Workstation emulation. Assume infinite memory.
  result = 0xffff;

#else
  // AVR-native.
  char stacktop;

  result = (&stacktop) - __malloc_heap_start;

  if (__brkval)
    result = (&stacktop) - __brkval;
#endif

  return (uint16_t) result;
}



//
// This is the end of the file.
