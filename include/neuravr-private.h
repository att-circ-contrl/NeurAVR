// Attention Circuits Control Laboratory - Atmel AVR firmware
// Common core header.
// Written by Christopher Thomas.
// Copyright (c) 2018 by Vanderbilt University. This work is licensed under
// the Creative Commons Attribution 4.0 International License.


//
// Variables


// Real-time clock variables.

extern volatile uint32_t rtc_timestamp;
extern void (*rtc_usercallback)(void);


// No shared GPIO variables.

// No shared ADC variables.

// No shared UART variables.



//
// Functions


// Real-time clock functions.

// No additional RTC functions.


// GPIO functions.

// No additional GPIO functions.


// ADC functions.

// This reinitializes the ADC buffer and conversion flags.
void ADC_ReInitBuffer(void);

// This checks ADC registers to see if a conversion is in progress.
bool ADC_IsADCBusy(void);

// This starts a conversion on the specified channel.
// Channel ID is 0..7.
void ADC_ReadFromChannel(uint8_t channel_id);

// This returns the value of the last converted sample.
// This is scaled to use the full 16-bit range.
uint16_t ADC_GetConversionResult(void);


// UART functions.

// This initializes buffer-handling. It should be called by UART_Init().
// The caller is responsible for any needed locking.
void UART_InitBuffers_ISR(void);

// These are buffer interaction handlers called from within ISRs.
void UART_HandleRecvChar_ISR(char recvchar);
bool UART_GetNextSendChar_ISR(char &sendchar);

// This is a transmission-start hook called after a string is queued.
// It re-enables need-character interrupts if they aren't aready enabled.
// The caller is responsible for any needed locking.
void UART_EnableTransmit_ISR(void);


// FIXME - ADC functions go here.


//
// This is the end of the file.
