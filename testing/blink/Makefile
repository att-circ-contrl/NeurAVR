# Attention Circuits Control Laboratory - Library Tests
# Makefile.
# Written by Christopher Thomas.
# Copyright (c) 2018 by Vanderbilt University. This work is licensed under
# the Creative Commons Attribution 4.0 International License.

#
# Configuration.

# Source files.

HDRS=	\

SRCS=	\
	blink.cpp

# Target name.
BIN=blink


# Compiler flags.
CFLAGSCOMMON=-Os -fno-exceptions -I../../include -L../../lib
CFLAGS328=$(CFLAGSCOMMON) -D__AVR_ATmega328P__ -mmcu=atmega328p
CFLAGS2560=$(CFLAGSCOMMON) -D__AVR_ATmega2560__ -mmcu=atmega2560

# Linking has to be done after compiling, so this is a separate variable.
LFLAGS328=-lneur-m328p
LFLAGS2560=-lneur-m2560


#
# Targets.

default: clean hex

elf: $(BIN)328.elf $(BIN)2560.elf
hex: $(BIN)328.hex $(BIN)2560.hex
asm: $(BIN)328.asm $(BIN)2560.asm

clean:
	rm -f $(BIN)*.elf
	rm -f $(BIN)*.hex
	rm -f $(BIN)*.asm

$(BIN)328.hex: $(BIN)328.elf
	avr-objcopy -j .text -j .data -O ihex $(BIN)328.elf $(BIN)328.hex

$(BIN)2560.hex: $(BIN)2560.elf
	avr-objcopy -j .text -j .data -O ihex $(BIN)2560.elf $(BIN)2560.hex

$(BIN)328.elf: $(SRCS) $(HDRS)
	avr-gcc $(CFLAGS328) -o $(BIN)328.elf $(SRCS) $(LFLAGS328)

$(BIN)2560.elf: $(SRCS) $(HDRS)
	avr-gcc $(CFLAGS2560) -o $(BIN)2560.elf $(SRCS) $(LFLAGS2560)

$(BIN)328.asm: $(BIN)328.elf
	avr-objdump -d $(BIN)328.elf > $(BIN)328.asm

$(BIN)2560.asm: $(BIN)2560.elf
	avr-objdump -d $(BIN)2560.elf > $(BIN)2560.asm

burn328: $(BIN)328.hex
	avrdude -c avrispv2 -P usb -p m328p -U flash:w:$(BIN)328.hex

ard328: $(BIN)328.hex
	avrdude -c stk500 -D -P /dev/ttyACM0 -p m328p -U flash:w:$(BIN)328.hex

burn2560: $(BIN)2560.hex
	avrdude -c avrispv2 -P usb -p m2560 -U flash:w:$(BIN)2560.hex

ard2560: $(BIN)2560.hex
	avrdude -c stk500 -D -P /dev/ttyACM0 -p m2560 -U flash:w:$(BIN)2560.hex

# FIXME - Setting the lock bits requires performing a chip erase!
# FIXME - Fuse settings are 2.7v brownout (needed for EEPROM),
# minimum boot loader size, boot from 0x0000 (not the boot loader),
# keep EEPROM during chip erase, external crystal, full swing, max
# startup delay, everything unlocked.
# FIXME - These are not Arduino-safe settings.
# NOTE - Only the least significant 3 bits of efuse are valid. The rest are 1.
fuses328:
	avrdude -c avrispv2 -P usb -p m328p -B 20 -e -u -U lock:w:0x3f:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U efuse:w:0x05:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U hfuse:w:0xd7:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U lfuse:w:0xf7:m
	avrdude -c avrispv2 -P usb -p m328p -B 1 -u -U lock:w:0x3f:m

# FIXME - Nominally arduino-stock, but difficult to test.
# These were snooped from an Arduino Uno with a new (ATmega32U4) serial
# translator.
# NOTE - Only the least significant 3 bits of efuse are valid. The rest are 1.
# 0x05 read means 0xfd written.
ardfuses328:
	avrdude -c avrispv2 -P usb -p m328p -B 20 -e -u -U lock:w:0x0f:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U efuse:w:0x05:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U hfuse:w:0xd6:m
	avrdude -c avrispv2 -P usb -p m328p -B 20 -u -U lfuse:w:0xff:m
	avrdude -c avrispv2 -P usb -p m328p -B 1 -u -U lock:w:0x0f:m

test:
	cu -l /dev/ttyACM0 -s 115200


#
# This is the end of the file.
