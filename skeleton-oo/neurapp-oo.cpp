// Attention Circuits Control Laboratory - Object-oriented firmware framework
// Top-level header.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
//
// Includes

#include <neuravr.h>
#include <neurapp-oo.h>

#if NEURAPP_DEBUG_AVAILABLE
// FIXME - We're using snprintf_P().
#include <stdio.h>
// FIXME - We're using strncpy_P().
#include <string.h>
#endif


//
//
//
// Private Constants

// Built-in commands.
neurapp_cmdname_t cmd_help  = { 'H', 'L', 'P' };
neurapp_cmdname_t cmd_ident = { 'I', 'D', 'Q' };
neurapp_cmdname_t cmd_reset = { 'I', 'N', 'I' };
neurapp_cmdname_t cmd_echo  = { 'E', 'C', 'H' };
#if NEURAPP_DEBUG_AVAILABLE
neurapp_cmdname_t cmd_debug_mem     = { 'Z', 'Z', 'M' };
neurapp_cmdname_t cmd_debug_evticks = { 'Z', 'Z', 'E' };
#endif

// Help screen for built-in commands.
const char neurapp_builtin_help[] PROGMEM =
  "Built-in commands:\r\n"
  "\r\n"
  " ?, HLP  :  Help screen.\r\n"
  "  ECH 1/0:  Start/stop echoing typed characters back to the host.\r\n"
  "  IDQ    :  Device identification string query.\r\n"
  "  INI    :  Reinitialize (reset clock and idle events).\r\n"
#if NEURAPP_DEBUG_AVAILABLE
  "\r\n"
  "Built-in debugging commands:\r\n"
  "\r\n"
  "  ZZM    :  Report the amount of free memory.\r\n"
  "  ZZE    :  Report accumulated timeslice overruns for event handlers.\r\n"
#endif
  ;



//
//
// Private Enums


enum parse_state_t
{
  PSTATE_PREAMBLE,
  PSTATE_OPCODE,
  PSTATE_FIRSTGAP,
  PSTATE_FIRSTARG,
  PSTATE_SECONDGAP,
  PSTATE_SECONDARG,
  PSTATE_TAIL,
  PSTATE_ERROR
};



//
//
// Classes


//
// Command/event handler - base class.
// Each of these is intended to manage one "feature" of the application,
// controlled by a group of user commands and interacting with the timer ISR
// and the polling/reporting loop.
// Member functions default to empty functions. Most are normally overridden.


// Returns a help screen describing handler-specific commands.

PGM_P NeurAppEvent_Base::GetHelpScreen(void)
{
  PGM_P result;

  result = PSTR("Command-specific help goes here.\r\n");

  return result;
}


// This performs one-time hardware initialization.

void NeurAppEvent_Base::InitHardware(void)
{
  // Default implementation: Do nothing.
}


// This performs internal state initialization. Multiple calls are ok.

void NeurAppEvent_Base::InitState(void)
{
  // Default implementation: Do nothing.
}


// This is called from the timer ISR.
// This should take less than one tick to complete.

void NeurAppEvent_Base::HandleTick_ISR(void)
{
  // Default implementation: Do nothing.
}


// High-priority polling code.
// This is called from the timer ISR and preempts normal code, but may
// take longer than one tick to complete.

void NeurAppEvent_Base::HandlePollHighPriority_ISR(void)
{
  // Default implementation: Do nothing.
}


// This is called to handle user commands.

void NeurAppEvent_Base::HandleCommand(uint8_t opcode,
  uint16_t arg1, uint16_t arg2)
{
  // Default implementation: Do nothing.
}


// This is called from within an atomic lock prior to report generation.
// It should copy any volatile data that we want to generate reports from.
// This implementation should be fast, so as not to tie up the lock.

void NeurAppEvent_Base::SaveReportState_Fast(void)
{
  // Default implementation: Do nothing.
}


// This is called from the polling loop to generate report text.

bool NeurAppEvent_Base::MakeReportString(neurapp_report_buf_t &buffer)
{
  // Default implementation: Do nothing.

  return false;
}


// This is called from the polling loop.
// It's intended to perform incremental processing on long-running tasks.

void NeurAppEvent_Base::HandlePolling(void)
{
  // Default implementation: Do nothing.
}



//
// Low-level command parser.
// This turns input strings into command/arg1/arg2 tuples.


// Constructor.

NeurApp_Parser::NeurApp_Parser(void)
{
  ResetState();
}


// Initializes state. This may be called multiple times.

void NeurApp_Parser::ResetState(void)
{
  int cidx;

  have_command = false;

  for (cidx = 0; cidx < NEURAPP_CMD_CHARS; cidx++)
    this_cmdname[cidx] = 0;

  this_arg1 = 0;
  this_arg2 = 0;
  argsfound = 0;
}


// Processes one line of input.
// Returns true if ok or empty, and false if parsing failed.

bool NeurApp_Parser::ParseInputLine(char *rawline)
{
  bool was_ok;
  parse_state_t state;
  int rawidx, opidx;
  char thischar;
  bool saw_question;
  bool is_letter, is_digit, is_white;
  uint16_t scratch;

  // Force state to known-clean values.
  ResetState();

  // Initialize parsing.
  was_ok = true;
  state = PSTATE_PREAMBLE;
  opidx = 0;
  saw_question = false;


  // Scan the input string.

  for (rawidx = 0; 0 != (thischar = rawline[rawidx]); rawidx++)
  {
    // Figure out what type of character this is.
    // Convert to upper case while we're at it.

    is_letter = false;
    is_digit = false;
    is_white = false;

    if ( ('a' <= thischar) && ('z' >= thischar) )
    {
      is_letter = true;
      thischar -= 'a';
      thischar += 'A';
    }
    else if ( ('A' <= thischar) && ('Z' >= thischar) )
      is_letter = true;
    else if ( ('0' <= thischar) && ('9' >= thischar) )
      is_digit = true;
    else if (' ' >= thischar)
      is_white = true;
    else if ('?' == thischar)
      saw_question = true;


    // Update parsing state.
    // Don't worry about character counts in this step.

    switch (state)
    {
      case PSTATE_PREAMBLE:
        if (is_letter)
          state = PSTATE_OPCODE;
        else if (!is_white)
          state = PSTATE_ERROR;
        break;

      case PSTATE_OPCODE:
        if (is_white)
          state = PSTATE_FIRSTGAP;
        else if (!is_letter)
          state = PSTATE_ERROR;
        break;

      case PSTATE_FIRSTGAP:
        if (is_digit)
          state = PSTATE_FIRSTARG;
        else if (!is_white)
          state = PSTATE_ERROR;
        break;

      case PSTATE_FIRSTARG:
        if (is_white)
          state = PSTATE_SECONDGAP;
        else if (!is_digit)
          state = PSTATE_ERROR;
        break;

      case PSTATE_SECONDGAP:
        if (is_digit)
          state = PSTATE_SECONDARG;
        else if (!is_white)
          state = PSTATE_ERROR;
        break;

      case PSTATE_SECONDARG:
        if (is_white)
          state = PSTATE_TAIL;
        else if (!is_digit)
          state = PSTATE_ERROR;
        break;

      case PSTATE_TAIL:
        if (!is_white)
          state = PSTATE_ERROR;
        break;

      default:
        // Already in error state.
        // Nothing to do.
        break;
    }


    // Now that we know what state we're in, update data.

    switch (state)
    {
      case PSTATE_OPCODE:
        have_command = true;
        if (opidx < NEURAPP_CMD_CHARS)
        {
          this_cmdname[opidx] = thischar;
          opidx++;
        }
        else
          state = PSTATE_ERROR;
        break;

      case PSTATE_FIRSTARG:
        argsfound = 1;
        this_arg1 *= 10;
        scratch = thischar;
        scratch -= '0';
        this_arg1 += scratch;
        break;

      case PSTATE_SECONDARG:
        argsfound = 2;
        this_arg2 *= 10;
        scratch = thischar;
        scratch -= '0';
        this_arg2 += scratch;
        break;

      default:
        // Whatever this is, there's nothing more to do with it.
        break;
    }
  }

  // If we wound up in the error state, report it as an error.
  if (PSTATE_ERROR == state)
    was_ok = false;


  // Special-case "?".
  // This will have generated an error during initial parsing.

  if (saw_question)
  {
    // Pretend we saw "HLP" with no arguments.

    ResetState();

    have_command = true;
    was_ok = true;

    for (opidx = 0; opidx < NEURAPP_CMD_CHARS; opidx++)
      this_cmdname[opidx] = cmd_help[opidx];
  }


  // Squash the output if we had an error.
  if (!was_ok)
    ResetState();


  // Done.
  return was_ok;
}


// Queries the most recent parsed command.
// Returns true if a command was parsed, false otherwise.
// Data is only copied if a new command was present.

bool NeurApp_Parser::WasNewCommand(neurapp_cmdname_t &cmd,
  uint16_t &arg1, uint16_t &arg2, int &argcount)
{
  bool result;
  int cidx;

  result = have_command;

  if (have_command)
  {
    have_command = false;

    for (cidx = 0; cidx < NEURAPP_CMD_CHARS; cidx++)
      cmd[cidx] = this_cmdname[cidx];

    arg1 = this_arg1;
    arg2 = this_arg2;
    argcount = argsfound;

    // State gets reset the next time parsing is performed.
  }

  return result;
}



//
// Top-level firmware implementation - base class.
// The application normally has exactly one of these.
// This can be used as-is, or "User" functions can be overridden.


// This returns true if two command names match and false otherwise.

bool NeurApp_Base::CommandMatch(neurapp_cmdname_t &first,
  neurapp_cmdname_t &second)
{
  bool result;
  int cidx;

  result = true;

  for (cidx = 0; cidx < NEURAPP_CMD_CHARS; cidx++)
    if (first[cidx] != second[cidx])
      result = false;

  return result;
}


// This writes a short "bad command, type HLP for help" message to the UART.

void NeurApp_Base::PrintShortHelp(char *rawline)
{
  int cidx;
  char thischar;

  UART_QueueSend_P(PSTR("Unrecognized command:  \""));

  // NOTE - We have no idea what's in the raw command string.
  // Render anything non-standard as hex.
  // NOTE - Printing character by character is slow, but that's ok.
  for (cidx = 0; 0 != (thischar = rawline[cidx]); cidx++)
  {
    if ((32 <= thischar) && (126 >= thischar))
    {
      // Seems legit.
      UART_PrintChar(thischar);
    }
    else
    {
      // Turn this into a hex string.
      UART_PrintChar('<');
      UART_PrintHex8(thischar);
      UART_PrintChar('>');
    }
  }

  UART_QueueSend_P(PSTR("\". Type \"?\" or \"HLP\" for help.\r\n"));
}


// User-defined only-on-reset initialization.

void NeurApp_Base::UserInitHardware(void)
{
  // Default implementation: Do nothing.
}


// User-defined happens-multiple-times initialization.

void NeurApp_Base::UserInitState(void)
{
  // Default implementation: Do nothing.
}


// User-defined timer ISR update code.
// This should take less than one tick to complete.

void NeurApp_Base::UserUpdateTimer_ISR(void)
{
  // Default implementation: Do nothing.
}


// User-defined high-priority polling events.
// This is called from the timer ISR and preempts normal code, but may
// take longer than one tick to complete.

void NeurApp_Base::UserPollHighPriority_ISR(void)
{
  // Default implementation: Do nothing.
}


// User-defined DoPolling() events.

void NeurApp_Base::UserUpdatePoll(void)
{
  // Default implementation: Do nothing.
}


// Constructor.

NeurApp_Base::NeurApp_Base(void)
{
  message_lut.identity_message = PSTR("BOGUS");
  message_lut.help_message_long = PSTR("BOGUS");
  parser.ResetState();
  echo_state = NEURAPP_DEFAULT_ECHO;

  event_lut = NULL;
}


// Default destructor is fine.


// One-time setup function.
// This should only be called once, after system reset.
// This calls all event "InitHardware" handlers.
// It also calls UserInitHardware().
// This automatically calls ReInitState().

void NeurApp_Base::DoInitialSetup(neurapp_messagedefs_t messagedefs,
    neurapp_event_handler_row_t *eventdefs)
{
  int hidx;

  // Copy the supplied structures.
  message_lut = messagedefs;
  event_lut = eventdefs;

  // Initialize ISR reentrant detection.
  in_isr = false;
  long_tasks_running = false;

  // Perform user-specified hardware initialization.
  UserInitHardware();

  // Walk through the event handler list and call "InitHardware" for each
  // handler.
  if (NULL != event_lut)
  {
    // NOTE - Duplicates may exist as adjacent entries. Special-case them.
    for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
    {
      if ( (1 > hidx)
        || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
        event_lut[hidx].handler->InitHardware();
    }
  }

  // Do a soft-reset of state.
  ReInitState();
}


// Soft-reset function.
// This may be called multiple times to reset system state.
// This calls all event "InitState" handlers.
// It also calls UserInitState().

void NeurApp_Base::ReInitState(void)
{
  int hidx;

  // This shouldn't be needed, but do it anyways.
  parser.ResetState();


  // Reset the report queue.

  report_read_ptr = 0;
  report_write_ptr = 0;
  report_count = 0;
  transmit_running = false;
  // Terminate strings just in case.
  for (hidx = 0; hidx < NEURAPP_REPORT_QUEUE_LENGTH; hidx++)
    reportqueue[hidx][0] = 0;

  // Force consistency by waiting for any in-progress transmission to finish.
  // FIXME - Is this safe to call here? It blocks.
  UART_WaitForSendDone();


  // Initialize debugging statistics.
#if NEURAPP_DEBUG_AVAILABLE
  for (hidx = 0; hidx < NEURAPP_DEBUG_EV_HANDLER_SLOTS; hidx++)
  {
    ev_handler_short_skipped_ticks[hidx] = 0;
    ev_handler_long_skipped_ticks[hidx] = 0;
  }
  skipped_ticks_short_total = 0;
  skipped_ticks_long_total = 0;
#endif


  // Perform user-specified re-initialization.
  UserInitState();


  // Walk through the event handler list and call "InitState" for each
  // handler.
  // NOTE - Duplicate handler pointers may exist. Multiple calls are okay,
  // so don't worry about special-casing them.

  if (NULL != event_lut)
  {
    for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
      event_lut[hidx].handler->InitState();
  }
}


// The timer ISR should call this.
// This calls all event "HandleTick_ISR" handlers, and also calls
// "UserUpdateTimer_ISR()".
// This likewise calls "HandlePollHighPriority_ISR()" and
// "UserPollHighPriority_ISR()". These can get interrupted, so they're
// allowed to take longer than one tick, but preempt non-interrupt tasks.

void NeurApp_Base::DoUpdate_ISR(void)
{
  int hidx;
#if NEURAPP_DEBUG_AVAILABLE
  uint32_t prevtime, thistime;
#endif


  // Short ISR tasks; these should happen every tick and complete quickly.
  // Handle nested/reentrant interrupts properly.

  if (in_isr)
  {
#if NEURAPP_DEBUG_AVAILABLE
    // We expect this to be zero.
    skipped_ticks_short_total++;
#endif
  }
  else
  {
    in_isr = true;

    NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)
    {
      // Walk through the event handler list and call "HandleTick_ISR" for
      // each handler.
      if (NULL != event_lut)
      {
#if NEURAPP_DEBUG_AVAILABLE
        // Check for time skew.
        thistime = Timer_Query_ISR();
#endif

        // NOTE - Duplicates may exist as adjacent entries. Special-case them.
        for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
        {
          if ( (1 > hidx)
            || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
            event_lut[hidx].handler->HandleTick_ISR();

#if NEURAPP_DEBUG_AVAILABLE
          // Check for time skew.
          prevtime = thistime;
          thistime = Timer_Query_ISR();
          if (hidx < NEURAPP_DEBUG_EV_HANDLER_SLOTS)
          {
            ev_handler_short_skipped_ticks[hidx] += thistime;
            ev_handler_short_skipped_ticks[hidx] -= prevtime;
          }
#endif
        }
      }

      // Perform user-specified timer ISR operations.
      UserUpdateTimer_ISR();
    }

    in_isr = false;
  }


  // High-priority polling tasks; these can take longer than a tick.
  // We still have to launch them from here so that they can preempt
  // lower-priority tasks.
  // Handle nested/reentrant interrupts properly.
  // Most of the time, these tasks will still be running.

  if (long_tasks_running)
  {
#if NEURAPP_DEBUG_AVAILABLE
    // We expect this to be nonzero.
    skipped_ticks_long_total++;
#endif
  }
  else
  {
    long_tasks_running = true;

    NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE)
    {
      // Walk through the event handler list and call
      // "HandlePollHighPriority_ISR" for each handler.
      if (NULL != event_lut)
      {
#if NEURAPP_DEBUG_AVAILABLE
        // Check for time skew.
        thistime = Timer_Query_ISR();
#endif

        // NOTE - Duplicates may exist as adjacent entries. Special-case them.
        for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
        {
          if ( (1 > hidx)
            || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
            event_lut[hidx].handler->HandlePollHighPriority_ISR();

#if NEURAPP_DEBUG_AVAILABLE
          // Check for time skew.
          prevtime = thistime;
          thistime = Timer_Query_ISR();
          if (hidx < NEURAPP_DEBUG_EV_HANDLER_SLOTS)
          {
            // We expect these to be nonzero.
            ev_handler_long_skipped_ticks[hidx] += thistime;
            ev_handler_long_skipped_ticks[hidx] -= prevtime;
          }
#endif
        }
      }

      // Perform user-specified high priority polling operations.
      UserPollHighPriority_ISR();
    }

    long_tasks_running = false;
  }
}


// The main application's polling loop should call this repeatedly.
// This checks for new commands, passes them to event handlers, checks
// for reports, and emits any generated reports.
// It also calls UserUpdatePoll().

void NeurApp_Base::DoPolling(void)
{
  char *thisline;
  neurapp_cmdname_t thiscommand;
  uint16_t arg1, arg2;
  int argcount;
  bool bad_command;
  int hidx, cidx;
  bool found;
  neurapp_cmd_list_row_t *cmdlist;
#if NEURAPP_DEBUG_AVAILABLE
  neurapp_report_buf_t debug_string;
#endif


  //
  // Check for new commands. Process the first one.

  thisline = UART_GetNextLine();
  if (NULL != thisline)
  {
    // Echo the command (if echoing).
    if (echo_state)
    {
      UART_QueueSend(thisline);
      UART_QueueSend_P(PSTR("\r\n"));
    }

    // Try to parse this line.
    if (parser.ParseInputLine(thisline))
    {
      // Parsing input succeeded; we either have a command or an empty line.
      if (parser.WasNewCommand(thiscommand, arg1, arg2, argcount))
      {
        // Check for built-in commands.

        bad_command = false;

        if (CommandMatch(thiscommand, cmd_help))
        {
          // Display the long-form help screen.

          // General banner.
          UART_QueueSend_P(PSTR("\r\n"));
          UART_QueueSend_P(message_lut.help_message_long);

          // Built-in commands.
          UART_QueueSend_P(PSTR("\r\n"));
          UART_QueueSend_P(neurapp_builtin_help);

          // Event handler specific commands.
          // NOTE - Duplicates may exist as adjacent entries.
          // Special-case them.
          for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
          {
            if ( (1 > hidx)
              || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
            {
              UART_QueueSend_P(PSTR("\r\n"));
              UART_QueueSend_P(event_lut[hidx].handler->GetHelpScreen());
            }
          }

          // Done.
          UART_QueueSend_P(PSTR("\r\n"));
        }
        else if (CommandMatch(thiscommand, cmd_ident))
        {
          UART_QueueSend_P(message_lut.identity_message);
        }
        else if (CommandMatch(thiscommand, cmd_reset))
        {
          ReInitState();
        }
        else if (CommandMatch(thiscommand, cmd_echo))
        {
          if (1 == argcount)
            echo_state = (arg1 != 0);
          else
            bad_command = true;
        }
#if NEURAPP_DEBUG_AVAILABLE
        else if (CommandMatch(thiscommand, cmd_debug_mem))
        {
          snprintf_P( debug_string, NEURAPP_REPORT_BUFFER_CHARS,
            PSTR("Available memory:  %u bytes\r\n"),
              (unsigned) MCU_GetFreeMemory() );
          UART_QueueSend(debug_string);
          UART_WaitForSendDone();
        }
        else if (CommandMatch(thiscommand, cmd_debug_evticks))
        {
          snprintf_P( debug_string, NEURAPP_REPORT_BUFFER_CHARS,
            PSTR("ISR skipped ticks: %10lu\r\n"),
            (unsigned long) skipped_ticks_short_total );
          UART_QueueSend(debug_string);
          UART_WaitForSendDone();

          for (hidx = 0; hidx < NEURAPP_DEBUG_EV_HANDLER_SLOTS; hidx++)
          {
            snprintf_P( debug_string, NEURAPP_REPORT_BUFFER_CHARS,
              PSTR("ISR handler %02u tick overruns:  %10lu\r\n"),
              (unsigned) hidx,
              (unsigned long) (ev_handler_short_skipped_ticks[hidx]) );
            UART_QueueSend(debug_string);
            UART_WaitForSendDone();
          }

          snprintf_P( debug_string, NEURAPP_REPORT_BUFFER_CHARS,
            PSTR("Priority poll skipped ticks: %10lu\r\n"),
            (unsigned long) skipped_ticks_long_total );
          UART_QueueSend(debug_string);
          UART_WaitForSendDone();

          for (hidx = 0; hidx < NEURAPP_DEBUG_EV_HANDLER_SLOTS; hidx++)
          {
            snprintf_P( debug_string, NEURAPP_REPORT_BUFFER_CHARS,
              PSTR("Priority poll handler %02u tick overruns:  %10lu\r\n"),
              (unsigned) hidx,
              (unsigned long) (ev_handler_long_skipped_ticks[hidx]) );
            UART_QueueSend(debug_string);
            UART_WaitForSendDone();
          }

          strncpy_P( debug_string, PSTR("End of skipped ticks.\r\n"),
            NEURAPP_REPORT_BUFFER_CHARS );
          UART_QueueSend(debug_string);
          UART_WaitForSendDone();
        }
#endif
        else
        {
          // This wasn't a built-in.
          // Walk through the event handler list, and send this command to
          // the appropriate handler.
          // NOTE - Duplicate handlers may exist as adjacent entries, with
          // different command lists. This is intentional.
          found = false;
          for (hidx = 0;
            (!found) && (NULL != event_lut[hidx].handler);
            hidx++)
          {
            cmdlist = event_lut[hidx].cmdlist;
            if (NULL != cmdlist)
            {
              for (cidx = 0;
                (!found) && (0 <= cmdlist[cidx].argcount);
                cidx++)
              {
                if (CommandMatch(thiscommand, cmdlist[cidx].name))
                {
                  found = true;
                  if (argcount == cmdlist[cidx].argcount)
                    // This looks like a valid command. Call the handler.
                    event_lut[hidx].handler->HandleCommand(
                      cmdlist[cidx].opcode, arg1, arg2);
                  else
                    // Wrong number of arguments.
                    bad_command = true;
                }
              }
            }
          }

          if (!found)
            bad_command = true;
        }

        // Check for unrecognized or malformed commands.
        if (bad_command)
          PrintShortHelp(thisline);
      }
    }
    else
    {
      // Parsing input failed. Print the short help message.
      PrintShortHelp(thisline);
    }

    // Whatever happened, we've finished with this line of input.
    UART_DoneWithLine();
  }


  //
  // Lock out interrupts and copy volatile event state.

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
      event_lut[hidx].handler->SaveReportState_Fast();
  }


  //
  // Generate reports.

  // First, send the next pending string if we can.

  if (0 < report_count)
  {
    if ( ! UART_IsSendInProgress() )
    {
      // If we were transmitting a string, make a note that we've finished.
      if (transmit_running)
      {
        transmit_running = false;
        report_count--;
        report_read_ptr++;
        if (report_read_ptr >= NEURAPP_REPORT_QUEUE_LENGTH)
          report_read_ptr = 0;
      }

      // If we still have a pending string, queue it to be transmitted.
      if (0 < report_count)
      {
        transmit_running = true;
        UART_QueueSend(reportqueue[report_read_ptr]);
      }
    }
  }

  // Second, queue new report strings.
  // NOTE - Remember that we can get multi-part messages for long reports!
  // NOTE - Don't block, here. If the queue is full, drop messages rather
  // than ignoring commands.

  for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
  {
    // NOTE - Duplicates may exist as adjacent entries. Special-case them.
    if ( (1 > hidx)
      || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
    {
      // Only ask for new messages while we have free slots for them.
      while ( (report_count < NEURAPP_REPORT_QUEUE_LENGTH)
        && event_lut[hidx].handler ->
          MakeReportString( reportqueue[report_write_ptr] ) )
      {
        // Make very sure this is NULL-terminated.
        reportqueue[report_write_ptr][NEURAPP_REPORT_BUFFER_CHARS-1] = 0;

        // Update the queue pointers.
        report_count++;
        report_write_ptr++;
        if (report_write_ptr >= NEURAPP_REPORT_QUEUE_LENGTH)
          report_write_ptr = 0;
      }
    }
  }


  //
  // Perform event handler polling operations.

  // NOTE - Duplicates may exist as adjacent entries. Special-case them.
  for (hidx = 0; NULL != event_lut[hidx].handler; hidx++)
  {
    if ( (1 > hidx)
      || (event_lut[hidx].handler != event_lut[hidx-1].handler) )
      event_lut[hidx].handler->HandlePolling();
  }


  //
  // Perform user-specified polling operations.

  UserUpdatePoll();
}


//
// This is the end of the file.
