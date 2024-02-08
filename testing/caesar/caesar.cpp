// Attention Circuits Control Laboratory - Library Tests
// Caesar's Code demo.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

//
// Includes

#include "neuravr.h"



//
// Macros

#define CPU_SPEED 16000000ul
#define LINK_BAUD 115200ul
//define LINK_BAUD 500000ul



//
// Functions



//
// Main Program

int main(void)
{
  uint32_t actual_baud;
  char *thisline;
  int idx;
  char thischar;
  bool done;

  MCU_Init();

  UART_Init(CPU_SPEED, LINK_BAUD);
  actual_baud = UART_QueryBaud();

  // FIXME - Diagnostics. Making sure filtering works properly.
//  UART_SetLineFiltering(true);

  done = false;
  thischar = 0;

  while (!done)
  {
    thisline = UART_GetNextLine();

    if (NULL != thisline)
    {
      for (idx = 0; 0 != (thischar = thisline[idx]); idx++)
      {
        if (('a' <= thischar) && ('z' >= thischar))
        {
          thischar -= 'a';
          thischar = (thischar + 13) % 26;
          thischar += 'a';
        }
        else if (('A' <= thischar) && ('Z' >= thischar))
        {
          thischar -= 'A';
          thischar = (thischar + 13) % 26;
          thischar += 'A';
        }
        else if ('?' == thischar)
        {
          UART_QueueSend_P(
            PSTR("\r\nCaesar's Code; type [esc][enter] to exit.\r\n"));

#if 1
// Formatted output test and baud rate reporting.
UART_QueueSend_P(PSTR("Actual baud rate:  "));
UART_PrintUInt(actual_baud);
// Test a non-pstr string.
UART_QueueSend((char *) " baud\r\n");
#if 1
// Test other output formats.
UART_QueueSend_P(PSTR("Formatted output tests:\r\n"));
{
   int32_t signbaud;
   signbaud = actual_baud;
   signbaud = -signbaud;
   UART_PrintSInt(signbaud);
}
UART_QueueSend_P(PSTR("\r\n"));
UART_PrintHex32(actual_baud);
UART_QueueSend_P(PSTR("\r\n"));
UART_PrintHex16((uint16_t) (actual_baud & 0xffff));
UART_QueueSend_P(PSTR("\r\n"));
UART_PrintHex8((uint8_t) (actual_baud & 0xff));
UART_QueueSend_P(PSTR("\r\n"));
#endif
#endif
        }
        else if (27 == thischar)
          done = true;

        if ((' ' <= thischar) && ('~' >= thischar))
        {
          UART_PrintChar(thischar);
        }
      }

      UART_DoneWithLine();

      UART_QueueSend_P(PSTR("\r\n"));
    }
  }

  UART_Init(0, 0);

  while (1) ;

  return 0;
}


//
// This is the end of the file.
