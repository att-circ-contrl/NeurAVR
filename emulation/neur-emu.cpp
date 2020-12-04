// Attention Circuits Control Laboratory - Atmel AVR firmware
// Compatibility layer for emulation on a workstation.
// Written by Christopher Thomas.
// Copyright (c) 2020 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

#include "neuravr.h"
#include "neuravr-private.h"

#include <iostream>
#include <string>
#include <list>
#include <map>



//
// Debugging


// Switches

#define TATTLE_ATOMIC 0
#define TATTLE_TIMER 0
#define TATTLE_UART 0
#define TATTLE_UART_CHARS 0


// Constants and global variables.

#if TATTLE_ATOMIC
std::map<std::thread::id,std::string> tid_lut;
#define TATTLE_ATOMIC_STAGGER_USECS 200000
#endif



//
// Private Enums

enum atomic_state_t
{
  ATOMIC_DONE = 0,
  ATOMIC_WANT_INTERRUPTS = 1,
  ATOMIC_WANT_NO_INTERRUPTS = 2
};



//
// Private Global Variables


// Variables for util/atomic.h.

std::mutex atomic_access_mutex;
std::mutex atomic_waiting_mutex;
volatile int atomic_owner_count = 0;
std::thread::id atomic_owner_id;


// Timer variables.

volatile bool have_timer_thread = false;
volatile bool timer_active = false;
volatile uint64_t clocks_per_tick = 0;
volatile uint64_t clocks_elapsed = 0;

// The argumentless constructor doesn't launch a thread.
// Use "threadvar = std::thread(function)" to do that.
std::thread timer_thread;


// GPIO variables.

uint8_t lastval_8 = 0;
uint16_t lastval_16 = 0;


// No ADC variables.


// UART variables.

volatile bool have_uart_threads = false;
volatile bool uart_active = false;
volatile uint32_t real_baud_rate = 0;
std::list<std::string> read_buffer;
std::list<std::string> write_buffer;

// The argumentless constructor doesn't launch a thread.
// Use "threadvar = std::thread(function)" to do that.
std::thread uart_reader;
std::thread uart_writer;
std::thread uart_feeder;



//
// Utility Functions


// FIXME - Ugly kludge for ATOMIC_BLOCK and NONATOMIC_BLOCK.
// The original is pretty ugly itself, so this is par for the course.


// This performs a fake "disable interrupts" operation, and returns a
// nonzero encoding of the desired interrupt state at the end of the block.

int ATOMIC_GetCli(bool save_old)
{
  int old_count;
  std::thread::id old_id;
  int result;

  // Check the bookkeeping state.

  atomic_access_mutex.lock();

  old_count = atomic_owner_count;
  old_id = atomic_owner_id;

  atomic_access_mutex.unlock();


  // Claim the lock if we don't already have it.
  // Note that the tid "!=" operator is going away in C++20, so use (!(==)).
  if ( (0 == old_count) || (!(std::this_thread::get_id() == old_id)) )
    atomic_waiting_mutex.lock();


  // We have the "waiting" mutex.

#if TATTLE_ATOMIC
  if ( 0 < tid_lut.count(std::this_thread::get_id()) )
    std::cerr << "-- Atomic lock acquired by thread "
      << tid_lut[std::this_thread::get_id()] << ".\n";
  else
    std::cerr << "-- Atomic lock acquired by thread "
      << std::this_thread::get_id() << ".\n";
#endif

  // Update the bookkeeping state.
  // This might have changed since we last saw it, but can't change
  // now that we have the "waiting" mutex (the write-acces mutex).

  atomic_access_mutex.lock();

  old_count = atomic_owner_count;

  atomic_owner_count++;
  atomic_owner_id = std::this_thread::get_id();

  atomic_access_mutex.unlock();


  // Return the appropriate value.
  // We either want to force interrupts on or keep the old state.
  result = ATOMIC_WANT_INTERRUPTS;
  if (save_old && (0 < old_count))
    result = ATOMIC_WANT_NO_INTERRUPTS;

  return result;
}


// This ends an atomic (CLI) block, returning 0.

int ATOMIC_ReleaseCli(int desired_state)
{
  int old_count;

  // This is only called at the end of a CLI block.
  // We own the "waiting" mutex, and are the only ones who can modify the
  // bookkeeping state.

#if TATTLE_ATOMIC
  if ( 0 < tid_lut.count(std::this_thread::get_id()) )
    std::cerr << "-- Atomic lock released by thread "
      << tid_lut[std::this_thread::get_id()] << ".\n";
  else
    std::cerr << "-- Atomic lock released by thread "
      << std::this_thread::get_id() << ".\n";
#endif

  // Update bookkeeping state.

  atomic_access_mutex.lock();

  atomic_owner_count--;
  // There's no safe value to reset the owner ID to. Leave it alone.
  // It's either our ID if count is still nonzero, or bogus if zero.

  old_count = atomic_owner_count;

  atomic_access_mutex.unlock();


  // FIXME - Ignore the "desired" state, since we're not supporting
  // NONATOMIC_BLOCK.


  // If we're at the outermost nesting layer, release the lock.
  // Otherwise, we still own it.
  if (0 == old_count)
    atomic_waiting_mutex.unlock();


  // Done.
  return ATOMIC_DONE;
}



//
// MCU Functions


// MCU initialization routine.
// Initializes the MCU to a known-good state.

void MCU_Init(void)
{
  // Global variables already have reasonable values.
  // Nothing to do!

#if TATTLE_ATOMIC
  tid_lut.insert(std::make_pair(std::this_thread::get_id(), "main"));
#endif
}



//
// Timer Functions


// RTC interrupt service routine.
// This updates the RTC timestamp, and optionally calls a user-provided
// function.

void Timer_ISR(void)
{
  // Grab the mutex before doing anything.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
#if TATTLE_TIMER
    std::cerr << "-- Timer ISR starts.\n";
#endif

    // Update emulator state.
    clocks_elapsed += clocks_per_tick;

    // Update neuravr state.
    rtc_timestamp++;
    if (NULL != rtc_usercallback)
      (*rtc_usercallback)();

#if TATTLE_TIMER
    std::cerr << "-- Timer ISR ends (" << clocks_elapsed << " clks, "
      << rtc_timestamp << " ticks).\n";
#endif
  }
}



// RTC interrupt thread.
// This spins forever, calling the ISR.

void Timer_ISRThread(void)
{
#if TATTLE_ATOMIC
  tid_lut.insert(std::make_pair(std::this_thread::get_id(), "timer"));
#endif
  // Spin, making sure to yield.
  while (1)
  {
    Timer_ISR();
    std::this_thread::yield();
  }
}



// RTC initialization routine.
// Unhooks all timers and initializes the RTC timer.
// An RTC rate of 0 disables the RTC.

void Timer_Init(uint32_t mcu_hz, uint32_t rtc_hz)
{
  bool new_timer_active;
  uint64_t new_clocks_per_tick;


  // Calculate parameters.

  new_timer_active = false;
  new_clocks_per_tick = 0;

  if (0 < rtc_hz)
  {
    // The only impact of RTC tick rate is the rate at which virtual time
    // increases per tick. We don't synchronize to wall-clock time.
    new_clocks_per_tick = mcu_hz / rtc_hz;
    new_timer_active = true;
  }


  // Grab the mutex and update the parameters.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    timer_active = new_timer_active;
    clocks_per_tick = new_clocks_per_tick;
  }


  // If we don't have a timer thread, start one.
  if (!have_timer_thread)
  {
#if TATTLE_ATOMIC
    usleep(TATTLE_ATOMIC_STAGGER_USECS);
#endif
    have_timer_thread = true;
    timer_thread = std::thread(Timer_ISRThread);
  }
}



//
// GPIO Functions


// 8-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO8_SelectOutputs(uint8_t output_mask)
{
  // Nothing to do.
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO8_WriteData(uint8_t output_data)
{
  lastval_8 = output_data;
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
  // Nothing to do.
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint8_t IO8_ReadData(void)
{
  // Nothing to do.
  return 0;
}


// 16-bit Digital GPIO functions.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.

void IO16_SelectOutputs(uint16_t output_mask)
{
  // Nothing to do.
}


// Asserts GPIO outputs. Only configured outputs are asserted.

void IO16_WriteData(uint16_t output_data)
{
  lastval_16 = output_data;
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
  // Nothing to do.
}


// Reads from GPIO inputs. Pins configured as outputs read as 0.

uint16_t IO16_ReadData(void)
{
  // Nothing to do.
  return 0;
}



//
// ADC Functions


// Initializes the ADC.
// This is one-time hardware initialization.
void ADC_Init(void)
{
  // Nothing to do.
  // FIXME - Maybe put a test pattern in here? It'd have to be fast, if so.
}


// This checks ADC registers to see if a conversion is in progress.
bool ADC_IsADCBusy(void)
{
  // Pretend to be infinitely fast.
  return false;
}


// This starts a conversion on the specified channel.
void ADC_ReadFromChannel(uint8_t channel_id)
{
  // Nothing to do.
}


// This returns the value of the last converted sample.
// This is scaled to use the full 16-bit range.
uint16_t ADC_GetConversionResult(void)
{
  // Give a mid-range value.
  return 0x8000;
}



//
// UART Functions


// Spins, reading from STDIN and saving to the read buffer.
// This is done regardless of whether the UART is "active" or not.

void UART_ReadThread(void)
{
  std::string thismsg;

#if TATTLE_ATOMIC
  tid_lut.insert(std::make_pair(std::this_thread::get_id(), "UART read"));
#endif

  // Spin, making sure to yield.
  while (1)
  {
    // FIXME - No provisions for input that isn't lines, or for forcing
    // unbuffered input.
    getline(std::cin, thismsg);

    // NOTE - c++ says getline() discards the '\n' delimiter.
    // Explicitly add it back.
    thismsg += '\n';

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
#if TATTLE_UART
      std::cerr << "[uart read] Got:\n" << thismsg;
#endif

      read_buffer.push_back(thismsg);
    }

    std::this_thread::yield();
  }
}



// Spins, writing new strings from the write buffer to STDOUT.
// This is done regardless of whether the UART is "active" or not.

void UART_WriteThread(void)
{
  std::string thismsg;
  bool have_message;

#if TATTLE_ATOMIC
  tid_lut.insert(std::make_pair(std::this_thread::get_id(), "UART write"));
#endif

  // Spin, making sure to yield.
  while (1)
  {
    have_message = false;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      if (!write_buffer.empty())
      {
        have_message = true;
        thismsg = write_buffer.front();
        write_buffer.pop_front();

#if TATTLE_UART
      std::cerr << "[uart write] Got:\n" << thismsg;
#endif
      }
    }

    if (have_message)
    {
      std::cout << thismsg;
      std::cout.flush();
    }

    std::this_thread::yield();
  }
}



// Spins, transferring characters between the neuravr framework and the
// I/O buffers.
// This only updates when the UART is "active".
// This calls void UART_HandleRecvChar_ISR(char thischar).
// This calls bool UART_GetNextSendChar_ISR(char &thischar).

void UART_FeederThread(void)
{
  std::string transmit_string;
  bool transmit_string_ready;
  std::string receive_string;
  std::string::iterator receive_idx;
  bool is_active;
  char thischar;

#if TATTLE_ATOMIC
  tid_lut.insert(std::make_pair(std::this_thread::get_id(), "UART relay"));
#endif

  transmit_string = "";
  transmit_string_ready = false;
  receive_string = "";
  receive_idx = receive_string.begin();

  // Spin, making sure to yield.
  while (1)
  {
    // Lock everything.
    // Interacting with the UART has to be locked, and the buffer-handling
    // functions of the NeurAVR framework are also intended to be called from
    // within a lock.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      //
      // Interact with the UART.

      is_active = uart_active;

      if (is_active)
      {
        // If we have a complete string queued to transmit, send it.
        if (transmit_string_ready)
        {
#if TATTLE_UART
          std::cerr << "[uart relay] Queued string for transmit:\n"
            << transmit_string;
#endif
          write_buffer.push_back(transmit_string);
          transmit_string_ready = false;
          transmit_string = "";
        }

        // If we've finished with the receive string we presently have,
        // get a new one if we can.
        if (receive_string.end() == receive_idx)
        {
          if (!read_buffer.empty())
          {
            receive_string = read_buffer.front();
            receive_idx = receive_string.begin();
            read_buffer.pop_front();
#if TATTLE_UART
          std::cerr << "[uart relay] Got a new received string:\n"
            << receive_string;
#endif
          }
        }
      }

      //
      // Interact with the neuravr framework.

      if (is_active)
      {
        // If we have characters in the read buffer, send one upstream.
        if (receive_string.end() != receive_idx)
        {
#if TATTLE_UART_CHARS
          std::cerr << "[uart relay] Handling received char \""
            << (*receive_idx) << "\".\n";
#endif
          UART_HandleRecvChar_ISR(*receive_idx);
          receive_idx++;
        }

        // If we have characters to transmit, append one to the write
        // buffer. If we have no more characters, queue it to send.
        thischar = 0;
        if (UART_GetNextSendChar_ISR(thischar))
        {
#if TATTLE_UART_CHARS
          std::cerr << "[uart relay] Handling transmitted char \""
            << thischar << "\".\n";
#endif
          // Add this character to the string.
          transmit_string.push_back(thischar);
        }
        else
        {
          // Assume this is the end of the string.
          // Only queue it if it has nonzero length.
          if (!transmit_string.empty())
            transmit_string_ready = true;
        }
      }

      // Finished this round of interactions.
    }

    // Yield.
    std::this_thread::yield();
  }
}



// Configures the primary UART for the specified baud rate.
// A baud rate of 0 turns it off.

void UART_Init(uint32_t mcu_hz, uint32_t baud_rate)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Update the real baud rate.
    real_baud_rate = baud_rate;

    // Update "active" flag.
    uart_active = false;
    if (0 < real_baud_rate)
      uart_active = true;
  }

  // If we don't have UART threads, start them.
  if (!have_uart_threads)
  {
    have_uart_threads = true;

#if TATTLE_ATOMIC
    usleep(TATTLE_ATOMIC_STAGGER_USECS);
#endif
    uart_reader = std::thread(UART_ReadThread);

#if TATTLE_ATOMIC
    usleep(TATTLE_ATOMIC_STAGGER_USECS);
#endif
    uart_writer = std::thread(UART_WriteThread);

#if TATTLE_ATOMIC
    usleep(TATTLE_ATOMIC_STAGGER_USECS);
#endif
    uart_feeder = std::thread(UART_FeederThread);
  }
}



// Returns the actual baud rate set, or 0 if the UART is off.

uint32_t UART_QueryBaud(void)
{
  return real_baud_rate;
}



// This is a transmission-start hook called after a string is queued.
// It re-enables need-character interrupts if they aren't already enabled.
// The caller is responsible for any needed locking.

void UART_EnableTransmit_ISR(void)
{
  // NOTE - Normally this wakes up "get next send character" polling.
  // For the emulated version, just poll all the time.

  // FIXME - Polling all the time might actually cause CPU load. Not sure.
}



//
// This is the end of the file.
