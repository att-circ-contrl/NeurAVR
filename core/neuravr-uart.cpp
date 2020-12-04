// Attention Circuits Control Laboratory - Atmel AVR firmware
// Common core - UART functions.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"

// FIXME - We're using snprintf().
#include <stdio.h>



//
// Macros

// A signed 32-bit number should take at most 11 digits to print.
#define INT_SCRATCH_CHARS 16



//
// Variables


// UART variables.

// Receive buffer. This is line-oriented and null-terminated.
char UART_recvlines[UART_LINE_COUNT * UART_LINE_SIZE];
int rowcount, oldestrow, newestrow;
int recvcharptr;

// User-supplied transmit buffer. This is a null-terminated string.
char *UART_transbuf;
int transcharptr;
bool trans_is_flash;

// Behavior flags.
bool uart_filter_empty_lines;

// Scratch string for printing.
char scratchstr[INT_SCRATCH_CHARS];



//
// Functions


// UART functions.


// Returns a pointer to the next complete line of input, or NULL if no
// complete line has been received yet.

char *UART_GetNextLine(void)
{
  char *result;

  result = NULL;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if (0 < rowcount)
    {
      // This is equivalent to &UART_recvlines[oldestrow * UART_LINE_SIZE].
      result = UART_recvlines + (oldestrow << UART_LINE_BITS);
    }

    // Don't adjust row counts for now - the entry has to remain valid.
  }

  return result;
}



// Tells the UART manager that we're done with the line we requested a
// pointer to.

void UART_DoneWithLine(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Get rid of the oldest line. We don't actually care if the user
    // asked for a pointer to it or not.
    if (0 < rowcount)
    {
      oldestrow = (oldestrow + 1) & (UART_LINE_COUNT - 1);
      rowcount--;
    }
  }
}



// Queues a string for UART transmission. This must be NULL-terminated.
// This blocks until any previous transmission has finished.

void UART_QueueSend(char *message)
{
  UART_WaitForSendDone();

  if (NULL != message)
  {
    // A lock shouldn't be necessary, but use it anyways.
    // It's possible that the very last character is still being transmitted,
    // which would generate a next-character request before we finish this.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      UART_transbuf = message;
      transcharptr = 0;
      trans_is_flash = false;

      UART_EnableTransmit_ISR();
    }
  }
}



// Queues a string from flash memory for UART transmission.
// Otherwise behaves as UART_QueueSend().

void UART_QueueSend_P(PGM_P message)
{
  UART_WaitForSendDone();

  if (NULL != message)
  {
    // A lock shouldn't be necessary, but use it anyways.
    // It's possible that the very last character is still being transmitted,
    // which would generate a next-character request before we finish this.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      UART_transbuf = (char *) message;
      transcharptr = 0;
      trans_is_flash = true;

      UART_EnableTransmit_ISR();
    }
  }
}



// Blocks until the transmission in progress (if any) completes.
// Granularity is several hundred clock cycles due to busy-wait padding.
// Interrupts are still handled during this time.

void UART_WaitForSendDone(void)
{
  while (UART_IsSendInProgress())
  {
    // Busy-wait so as to not hammer ATOMIC_BLOCK().
    // This waits about 600 clock cycles (40 us at 16 MHz).
    _delay_loop_1(200);
  }
}



// Queries whether or not a transmission is in progress.

bool UART_IsSendInProgress(void)
{
  bool result;

  result = false;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    if (NULL != UART_transbuf)
      result = true;
  }

  return result;
}



// This initializes buffer-handling. It should be called by UART_Init().
// The caller is responsible for any needed locking.

void UART_InitBuffers_ISR(void)
{
  // Initialize all received rows to null-terminated empty strings.
  for (rowcount = 0; rowcount < UART_LINE_COUNT; rowcount++)
    UART_recvlines[rowcount << UART_LINE_BITS] = 0;

  // Initialize tracking variables.
  rowcount = 0;
  oldestrow = 0;
  newestrow = 0;
  recvcharptr = 0;

  // Initialize transmit buffer.
  UART_transbuf = NULL;
  transcharptr = 0;
  trans_is_flash = false;

  // Initialize behavior state.
  uart_filter_empty_lines = false;
}



// This handles a received character.

void UART_HandleRecvChar_ISR(char recvchar)
{
  static bool saw_cr = false;

  // Process this character.
  if (saw_cr && ('\n' == recvchar))
  {
    // Do nothing else.
    // Ignore the "LF" in "CRLF" even if filtering is off.
  }
  else if (('\n' == recvchar) || ('\r' == recvchar))
  {
    // End-of-line marker.

    if (uart_filter_empty_lines && (0 == recvcharptr))
    {
      // If the line we just finished is empty, and we're filtering empty
      // lines, do nothing (overwrite it with the next line).
      // This eats vertical whitespace in documents and also means "press
      // enter to continue" doesn't work. It's still helpful when operating
      // at wire speed, as it makes it less likely our input buffer will jam.
    }
    else
    {
      // We either have a non-empty line, or want to keep empties.

      // Terminate this line.
      if (recvcharptr >= UART_LINE_SIZE)
        recvcharptr = UART_LINE_SIZE - 1;
      UART_recvlines[(newestrow << UART_LINE_BITS) + recvcharptr] = 0;

      // Advance to the next line. If we're full, stay on this line.
      // NOTE - "rowcount" is the number of _completed_ lines. We always
      // have one _incomplete_ line we're working on, so the true cap is
      // UART_LINE_COUNT - 1, not UART_LINE_COUNT.
      if (rowcount < (UART_LINE_COUNT - 1))
      {
        newestrow = (newestrow + 1) & (UART_LINE_COUNT - 1);
        rowcount++;
      }

      // New line or the same line, terminate it and initialize our
      // character pointer.
      UART_recvlines[newestrow << UART_LINE_BITS] = 0;
      recvcharptr = 0;
    }
  }
  else
  {
    // Add this character to the line.
    // If we're at the end of the line, silently drop this character.
    // FIXME - Don't filter control characters or high-ascii.
    // Let the application do that. It might _want_ them.
    if (recvcharptr < UART_LINE_SIZE)
    {
      UART_recvlines[(newestrow << UART_LINE_BITS) + recvcharptr] = recvchar;
      recvcharptr++;
      // Don't terminate this string. We do that on end-of-line.
    }
  }

  // Update our CRLF tracking.
  if ('\r' == recvchar)
    saw_cr = true;
  else
    saw_cr = false;
}



// This provides the next character to transmit, if any.

bool UART_GetNextSendChar_ISR(char &sendchar)
{
  bool result;
  char thischar;

  result = false;
  thischar = 0;

  if (NULL != UART_transbuf)
  {
    if (trans_is_flash)
#if USE_FAR_FLASH_POINTERS
      // FIXME - This presumes that transcharptr will get promoted to 32 bits!
      thischar = pgm_read_byte_far(UART_transbuf + transcharptr);
#else
      thischar = pgm_read_byte_near(UART_transbuf + transcharptr);
#endif
    else
      thischar = UART_transbuf[transcharptr];

    if (0 == thischar)
    {
      // End of string. Re-initialize buffer and report failure.
      UART_transbuf = NULL;
      transcharptr = 0;
      trans_is_flash = 0;
      result = false;
    }
    else
    {
      // We succeeded in fetching a character.
      transcharptr++;
      result = true;
    }
  }

  sendchar = thischar;
  return result;
}



// Turns empty line filtering on or off.
// This saves buffer space but feels less interactive to users.

void UART_SetLineFiltering(bool new_state)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    uart_filter_empty_lines = new_state;
  }
}



// Formatted printing functions.

// Single-character output.

void UART_PrintChar(char value)
{
  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  scratchstr[0] = value;
  scratchstr[1] = 0;

  UART_QueueSend(scratchstr);
}


// Formatted integers.
// FIXME - These use snprintf(), which is slow and uses malloc().
// Live with it, because writing my own is error-prone.

void UART_PrintUInt(uint32_t value)
{
  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  // This always null-terminates, since we have extra space.
  // It's supposed to no matter what, but extra space helps.
  // NOTE - Need the explicit cast for compiling the emulated version.
  snprintf(scratchstr, INT_SCRATCH_CHARS, "%lu", (unsigned long) value);

  UART_QueueSend(scratchstr);
}


void UART_PrintSInt(int32_t value)
{
  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  // This always null-terminates, since we have extra space.
  // It's supposed to no matter what, but extra space helps.
  // NOTE - Need the explicit cast for compiling the emulated version.
  snprintf(scratchstr, INT_SCRATCH_CHARS, "%ld", (long) value);

  UART_QueueSend(scratchstr);
}


void UART_PrintHex32(uint32_t value)
{
  int idx;

  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  // Use the fast-printing routine.
  for (idx = 0; idx < INT_SCRATCH_CHARS; idx++)
    scratchstr[idx] = 0;
  UTIL_WriteHex(scratchstr, value, 8);

  UART_QueueSend(scratchstr);
}


void UART_PrintHex16(uint16_t value)
{
  int idx;

  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  // Use the fast-printing routine.
  for (idx = 0; idx < INT_SCRATCH_CHARS; idx++)
    scratchstr[idx] = 0;
  UTIL_WriteHex(scratchstr, value, 4);

  UART_QueueSend(scratchstr);
}


void UART_PrintHex8(uint8_t value)
{
  int idx;

  // Make sure we aren't still using the scratch string.
  UART_WaitForSendDone();

  // Use the fast-printing routine.
  for (idx = 0; idx < INT_SCRATCH_CHARS; idx++)
    scratchstr[idx] = 0;
  UTIL_WriteHex(scratchstr, value, 2);

  UART_QueueSend(scratchstr);
}



//
// This is the end of the file.
