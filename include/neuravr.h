// Attention Circuits Control Laboratory - Atmel AVR firmware
// Top-level header.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Includes

// Common things we'll always want to include.
#include <stdint.h>
#include <stdlib.h>

#ifdef NEUREMU

// Emulation library.
#include "neur-emu.h"

#else

// AVR libraries.
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay_basic.h>

#endif


// Architecture-specific headers.

#ifdef __AVR_ATmega2560__
#include "neur-m2560.h"
#endif

#ifdef __AVR_ATmega328P__
#include "neur-m328p.h"
#endif

#ifdef __AVR_ATmega32U4__
#include "neur-m32u4.h"
#endif


//
// Functions


// Initialization functions.

// Initializes the MCU to a known-good state.
void MCU_Init(void);


// Real-time clock functions.

// Unhooks all timers and initializes the RTC timer.
// An RTC rate of 0 disables the RTC.
void Timer_Init(uint32_t mcu_hz, uint32_t rtc_hz);

// Clears the RTC timestamp.
void Timer_Reset(void);

// Reads the real-time clock timestamp.
uint32_t Timer_Query(void);

// Reads the real-time clock timestamp while within an ISR or other
// locked code. This avoids an ATOMIC_BLOCK call.
uint32_t Timer_Query_ISR(void);

// Specifies a user-defined function to call during timer interrupts.
// _All_ interrupts, not just timer interrupts, are disabled while this
// runs, so it has to return very quickly.
void Timer_RegisterCallback(void (*callback)(void));


// 8-bit Digital GPIO functions.
// The 328p and 2560 both have this bank.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.
void IO8_SelectOutputs(uint8_t output_mask);

// Asserts GPIO outputs. Only configured outputs are asserted.
void IO8_WriteData(uint8_t output_data);

// Returns the last written value. This lets the user set/clear bits
// without disturbing bits that are to remain the same.
uint8_t IO8_GetOutputValue(void);

// Enables pull-ups on selected GPIO lines. Only configured inputs have
// pull-ups. 1 = pull-up, 0 = floating.
void IO8_SetPullups(uint8_t pullup_mask);

// Reads from GPIO inputs. Pins configured as outputs read as 0.
uint8_t IO8_ReadData(void);


// 16-bit Digital GPIO functions.
// The 2560 has this bank in addtion to the 8-bit bank.
// Calling these functions on the 328p is safe but does nothing.

// Configures input and output GPIO lines. 1 = output, 0 = input.
// NOTE - Pull-up state should be set immediately after this.
void IO16_SelectOutputs(uint16_t output_mask);

// Asserts GPIO outputs. Only configured outputs are asserted.
void IO16_WriteData(uint16_t output_data);

// Returns the last written value. This lets the user set/clear bits
// without disturbing bits that are to remain the same.
uint16_t IO16_GetOutputValue(void);

// Enables pull-ups on selected GPIO lines. Only configured inputs have
// pull-ups. 1 = pull-up, 0 = floating.
void IO16_SetPullups(uint16_t pullup_mask);

// Reads from GPIO inputs. Pins configured as outputs read as 0.
uint16_t IO16_ReadData(void);


// Analog to digital converter functions.
// The 328p has 6 channels; the 2560 has 8 (we don't use 8..15).
// Accessing an invalid channel is safe but does nothing.

// Initializes the ADC, selecting unipolar input and an internal reference.
// The 2560 has a 2.56V reference. The 328p has a 1.1V reference.
// This is one-time hardware initialization.
void ADC_Init(void);

// Performs housekeeping polling for the ADC.
// This is typically called from the timer interrupt.
// At 16 MHz, polling at 8 kHz is optimal (conversions take 1700 clocks).
void ADC_HousekeepingPoll(void);

// Queues conversion of analog signals on the specified ADC channels.
// This is ignored if a previous conversion is still in progress.
// Any unread pending data is discarded.
void ADC_StartConversion(uint8_t channel_mask);

// Returns true if the queued conversion has completed and has unread data.
bool ADC_IsDataReady(void);

// Blocks until the queued conversion (if any) completes.
// Granularity is several hundred clock cycles due to busy-wait padding.
// Interrupts are still handled during this time.
void ADC_WaitForData(void);

// Reads data for the next completed but unread analog sample.
// Returns true if unread data was present and false otherwise.
// Channel ID is set to 0..7; this channel's mask is 1 << channel_id.
bool ADC_ReadPendingSample(uint16_t &data, uint8_t &channel_id);


// UART functions.

// Configures the primary UART for the specified baud rate.
// A baud rate of 0 turns it off.
// Which UART is primary is architecture-dependent.
void UART_Init(uint32_t mcu_hz, uint32_t baud_rate);

// Returns the actual baud rate set, or 0 if the UART is off.
uint32_t UART_QueryBaud(void);

// Returns a pointer to the next complete line of input, or NULL if no
// complete line has been received yet.
char *UART_GetNextLine(void);

// Tells the UART manager that we're done with the line we requested a
// pointer to.
void UART_DoneWithLine(void);

// Queues a string for UART transmission. This must be NULL-terminated.
// This blocks until any previous transmission has finished.
void UART_QueueSend(char *message);

// Queues a string from flash memory for UART transmission.
// Otherwise behaves as UART_QueueSend().
void UART_QueueSend_P(PGM_P message);

// Blocks until the transmission in progress (if any) completes.
// Granularity is several hundred clock cycles due to busy-wait padding.
// Interrupts are still handled during this time.
void UART_WaitForSendDone(void);

// Queries whether or not a transmission is in progress.
bool UART_IsSendInProgress(void);

// Turns empty line filtering on or off.
// This saves buffer space but feels less interactive to users.
void UART_SetLineFiltering(bool new_state);


// Formatted printing functions.

// Single-character output.
void UART_PrintChar(char value);

// Formatted integers.
void UART_PrintUInt(uint32_t value);
void UART_PrintSInt(int32_t value);
void UART_PrintHex32(uint32_t value);
void UART_PrintHex16(uint16_t value);
void UART_PrintHex8(uint8_t value);


// Debugging functions.

uint16_t MCU_GetFreeMemory(void);


// Utility functions not tied to a particular module.

// Fast but unsafe printing functions (no bounds checking).

// This renders an integer as hexidecimal digits into a string.
void UTIL_WriteHex(char *buffer, uint32_t data, uint8_t digits);


//
// This is the end of the file.
