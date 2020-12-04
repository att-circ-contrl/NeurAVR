// Attention Circuits Control Laboratory - Atmel AVR firmware
// Compatibility header for emulation on a workstation.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include <unistd.h>

#include <mutex>
#include <thread>



//
// Macros - avr/pgmspace.h

// Nothing special about "program memory" in emulation.
#define PSTR(X) (X)
#define PROGMEM /* Do nothing. */

// Program memory pointers and pointer access.
#define PGM_P const char *
#define pgm_read_byte_near(X) (*(X))
#define pgm_read_byte_far(X) (*(X))

// Various _P functions revert to their normal versions.
#define strncpy_P strncpy
#define snprintf_P snprintf



//
// Macros - avr/interrupt.h

// FIXME - Count on the user to stub out vector table declarations instead.



//
// Macros - util/atomic.h


// FIXME - This is a pretty ugly kludge.
// On the other hand, the original is kludged in a very similar way.


// Translate "Restore/"Force" macros into a "keep original" flag.
// Whether we're forcing on or forcing off depends on which function we call.

#define ATOMIC_RESTORESTATE true
#define ATOMIC_FORCEON false
#define NONATOMIC_RESTORESTATE true
#define NONATOMIC_FORCEOFF false


// ATOMIC_BLOCK and NONATOMIC_BLOCK abuse for() syntax to do their thing.

#define ATOMIC_BLOCK(X) \
for ( int ATOMIC_idx = ATOMIC_GetCli(X); \
  ATOMIC_idx != 0; \
  ATOMIC_idx = ATOMIC_ReleaseCli(ATOMIC_idx) )

// FIXME - Not supporting NONATOMIC_BLOCK.

#define NONATOMIC_BLOCK(X)



//
// Functions - util/atomic.h

// FIXME - Ugly kludge for ATOMIC_BLOCK and NONATOMIC_BLOCK.
// The original is pretty ugly itself, so this is par for the course.

// This performs a fake "disable interrupts" operation, and returns a
// nonzero encoding of the desired interrupt state at the end of the block.
int ATOMIC_GetCli(bool save_old);

// This ends an atomic (CLI) block, returning 0.
int ATOMIC_ReleaseCli(int desired_state);



//
// Macros - util/delay_basic.h

// This delays for up to 256 iterations, at 3 clocks each (3/16 us).
// FIXME - Kludge to (X/4 + 1) us.
#define _delay_loop_1(X) usleep(1 + ((X) >> 2))

// This delays for up to 64k iterations, at 4 clocks each (1/4 us).
// FIXME - Kludge to (X/4 + 1) us. This is actually pretty accurate.
#define _delay_loop_2(X) usleep(1 + ((X) >> 2))


//
// This is the end of the file.
