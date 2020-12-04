// Attention Circuits Control Laboratory - Object-oriented firmware framework
// Top-level header.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include <neuravr.h>



//
// Macros

// Command string length.
// Command mnemonics are fixed-length sequences of capital letters.
#define NEURAPP_CMD_CHARS 3

// Default echo state.
#define NEURAPP_DEFAULT_ECHO true

// Event report buffer size.
// Longer reports get passed in several pieces.
// Making this a bit longer than one standard line, for CRLFs and so forth.
#define NEURAPP_REPORT_BUFFER_CHARS 90

// Number of outgoing message buffers.
// We can queue up to this many messages before blocking.
// This should be small. It does not have to be a power of 2.
#define NEURAPP_REPORT_QUEUE_LENGTH 4

// Enable/disable debugging commands (profiling etc).
#define NEURAPP_DEBUG_AVAILABLE 1

// Number of reporting slots for profiling event handlers.
// This is the maximum number of handlers that we record statistics for.
#define NEURAPP_DEBUG_EV_HANDLER_SLOTS 16



//
// Typedefs


// Report string type.
typedef char neurapp_report_buf_t[NEURAPP_REPORT_BUFFER_CHARS];


// Command string type.
// Command mnemonics are fixed-length sequences of capital letters.
typedef char neurapp_cmdname_t[NEURAPP_CMD_CHARS];


// Command lookup table row type.
// Terminated by a negative argument count.

typedef struct
{
  // We can't initialize command names by value, so do it by reference.
  neurapp_cmdname_t &name;
  uint8_t opcode;
  int argcount;
} neurapp_cmd_list_row_t;


// Event handler lookup table row type.
// Terminated by NULL event handler.
// NOTE - The same handler may be listed multiple times with different
// command lists (this is how child classes implement parents' commands).
// Those handlers must be in adjacent entries to be recognized as duplicates.
// If they aren't, InitHardware() and HandleTick_ISR() will be called too
// often.

typedef struct
{
  // Event handler associated with this list of commands.
  class NeurAppEvent_Base *handler;
  // Command list, terminated by a negative argument count.
  neurapp_cmd_list_row_t *cmdlist;
} neurapp_event_handler_row_t;


// Structure containing message definitions the framework needs.
// These are strings stored in program memory, not SRAM.
// Define them with: PSTR("string text here") in local scope, or
// const char stringname[] PROGMEM = "string text here" in global scope.

typedef struct
{
  // String returned by the "IDQ" command.
  PGM_P identity_message;

  // General-purpose help banner returned before other help messages.
  PGM_P help_message_long;
} neurapp_messagedefs_t;



//
// Classes


// Command/event handler - base class.
// Each of these is intended to manage one "feature" of the application,
// controlled by a group of user commands and interacting with the timer ISR
// and the polling/reporting loop.
// Member functions default to empty functions. Most are normally overridden.

class NeurAppEvent_Base
{
protected:
  // Derived classes should put state variables here.

public:
  // Default constructor and destructor are fine.

  // Returns a help screen describing handler-specific commands.
  virtual PGM_P GetHelpScreen(void);

  // This performs one-time hardware initialization.
  virtual void InitHardware(void);
  // This performs internal state initialization. Multiple calls are ok.
  virtual void InitState(void);

  // This is called from the timer ISR.
  // This should take less than one tick to complete.
  virtual void HandleTick_ISR(void);
  // High-priority polling code.
  // This is called from the timer ISR and preempts normal code, but may
  // take longer than one tick to complete.
  virtual void HandlePollHighPriority_ISR(void);

  // This is called to handle user commands.
  // Opcodes with fewer than two arguments have extra args set to zero.
  virtual void HandleCommand(uint8_t opcode, uint16_t arg1, uint16_t arg2);

  // This is called from within an atomic lock prior to report generation.
  // It should copy any volatile data that we want to generate reports from.
  // This implementation should be fast, so as not to tie up the lock.
  virtual void SaveReportState_Fast(void);
  // This is called from the polling loop to generate report text.
  // It returns true if an event report was generated, false otherwise.
  // NOTE - Any report generated _must_ be null-terminated!
  // NOTE - Buffer overflows are a Bad Thing. Take precautions.
  virtual bool MakeReportString(neurapp_report_buf_t &buffer);

  // This is called from the polling loop.
  // It's intended to perform incremental processing on long-running tasks.
  // This doesn't have any return time guarantee, but delays of 1 ms or more
  // will slow down command processing and reporting.
  virtual void HandlePolling(void);
};



// Low-level command parser.
// This turns input strings into command/arg1/arg2 tuples.

class NeurApp_Parser
{
protected:
  bool have_command;
  neurapp_cmdname_t this_cmdname;
  uint16_t this_arg1, this_arg2;
  int argsfound;

public:
  NeurApp_Parser(void);
  // Default destructor is fine.

  // Initializes state. This may be called multiple times.
  void ResetState(void);
  // Processes one line of input.
  // Returns true if ok or empty, and false if parsing failed.
  bool ParseInputLine(char *rawline);

  // Queries the most recent parsed command.
  // Returns true if a command was parsed, false otherwise.
  // Data is only copied if a new command was present.
  bool WasNewCommand(neurapp_cmdname_t &cmd,
    uint16_t &arg1, uint16_t &arg2, int &argcount);
};



// Top-level firmware implementation - base class.
// The application normally has exactly one of these.
// This can be used as-is, or "User" functions can be overridden.

class NeurApp_Base
{
protected:

  // Configuration information.
  neurapp_messagedefs_t message_lut;
  neurapp_event_handler_row_t *event_lut;

  // State.
  NeurApp_Parser parser;
  bool echo_state;

  // Outgoing message buffers.
  int report_read_ptr, report_write_ptr, report_count;
  bool transmit_running;
  neurapp_report_buf_t reportqueue[NEURAPP_REPORT_QUEUE_LENGTH];

  // Debugging/profiling buffers.
#if NEURAPP_DEBUG_AVAILABLE
  uint32_t ev_handler_short_skipped_ticks[NEURAPP_DEBUG_EV_HANDLER_SLOTS];
  uint32_t skipped_ticks_short_total;
  uint32_t ev_handler_long_skipped_ticks[NEURAPP_DEBUG_EV_HANDLER_SLOTS];
  uint32_t skipped_ticks_long_total;
#endif

  // Timer interrupt management.
  // This is for handling nested/reentrant interrupts properly.
  volatile bool in_isr;
  volatile bool long_tasks_running;


  //
  // Private utility functions.

  // This returns true if two command names match and false otherwise.
  bool CommandMatch(neurapp_cmdname_t &first, neurapp_cmdname_t &second);

  // This writes a short "bad command, type HLP for help" message to the UART.
  void PrintShortHelp(char *rawline);


  //
  // User-defined initialization functions.
  // These default to empty functions.

  // User-defined only-on-reset initialization.
  virtual void UserInitHardware(void);
  // User-defined happens-multiple-times initialization.
  virtual void UserInitState(void);


  //
  // User-defined application logic functions.
  // These default to empty functions.

  // User-defined timer ISR update code.
  // This should take less than one tick to complete.
  virtual void UserUpdateTimer_ISR(void);
  // User-defined high-priority polling code.
  // This is called from the timer ISR and preempts normal code, but may
  // take longer than one tick to complete.
  virtual void UserPollHighPriority_ISR(void);
  // User-defined DoPolling() events.
  virtual void UserUpdatePoll(void);


public:

  NeurApp_Base(void);
  // Default destructor is fine.


  //
  // Public built-in functions.

  // There should be no reason to override these - override the "user"
  // private functions instead.


  // One-time setup function.
  // This should only be called once, after system reset.
  // This calls all event "InitHardware" handlers.
  // It also calls UserInitHardware().
  // This automatically calls ReInitState().
  void DoInitialSetup(neurapp_messagedefs_t messagedefs,
    neurapp_event_handler_row_t *eventdefs);

  // Soft-reset function.
  // This may be called multiple times to reset system state.
  // This calls all event "InitState" handlers.
  // It also calls UserInitState().
  void ReInitState(void);

  // The timer ISR should call this.
  // This calls all event "HandleTick_ISR" handlers, and also calls
  // "UserUpdateTimer_ISR()".
  // This likewise calls "HandlePollHighPriority_ISR()" and
  // "UserPollHighPriority_ISR()". These can get interrupted, so they're
  // allowed to take longer than one tick, but preempt non-interrupt tasks.
  void DoUpdate_ISR(void);

  // The main application's polling loop should call this repeatedly.
  // This checks for new commands, passes them to event handlers, checks
  // for reports, and emits any generated reports.
  // It also calls UserUpdatePoll().
  void DoPolling(void);
};


//
// This is the end of the file.
