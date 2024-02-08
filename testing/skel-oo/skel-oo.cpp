// Attention Circuits Control Laboratory - Library Tests
// Object-oriented application skeleton test.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.

//
// Includes

#include "neuravr.h"
#include "neurapp-oo.h"



//
// Macros

#define CPU_SPEED 16000000ul
#define RTC_TICKS_PER_SECOND 10000ul
#define LINK_BAUD 115200ul

#define DEVICETYPE "Test Device"
#define DEVICESUBTYPE "v1"
#define VERSION_STR "20200305"


//
// Constants

neurapp_event_handler_row_t event_lut[] =
{
  { NULL, NULL }
};


// FIXME - PSTR() only works inside a function. This is equivalent.
const char versionstr[] PROGMEM =
  "devicetype: " DEVICETYPE
  "  subtype: " DEVICESUBTYPE
  "  revision: " VERSION_STR
  "\r\n";

const char helpscreenstr[] PROGMEM =
  "Help banner goes here.\r\n";

neurapp_messagedefs_t messages =
{
  versionstr,
  helpscreenstr
};



//
// Class Declarations

class TestApp : public NeurApp_Base
{
protected:
  void UserInitHardware(void);
  void UserInitEvents(void);
  void UserUpdateTimer_ISR(void);
  void UserUpdatePoll(void);
};



//
// Class Implementations


void TestApp::UserInitHardware(void)
{
}


void TestApp::UserInitEvents(void)
{
}


void TestApp::UserUpdateTimer_ISR(void)
{
}


void TestApp::UserUpdatePoll(void)
{
}



//
// Global Variables

TestApp test_application;



//
// Functions

void TimerCallback(void)
{
  test_application.DoUpdate_ISR();
}



//
// Main Program

int main(void)
{
  MCU_Init();

  UART_Init(CPU_SPEED, LINK_BAUD);

  test_application.DoInitialSetup(messages, event_lut);

  // Do this as the _last_ part of setup, as it starts timer ISR calls.
  Timer_Init(CPU_SPEED, RTC_TICKS_PER_SECOND);
  Timer_RegisterCallback(&TimerCallback);

  while (1)
  {
    test_application.DoPolling();
  }

  // We should never reach here.
  // Report success.
  return 0;
}


//
// This is the end of the file.
