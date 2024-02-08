# Attention Circuits Control Laboratory - Atmel AVR firmware
# README documentation.
Written by Christopher Thomas.


## Overview

This is intended to be a bare-bones hardware library for building AVR firmware
on without having to use the Arduino libraries or IDE.

Two main components are provided:

* A set of low-level libraries (`libneur-XXXX`).

* An object-oriented application framework (`libneurapp-XXXX`).

These are compiled for two targets:

* Various AVR architectures (the `-mXXXX` libraries).

* Workstation targets that are configured to resemble the AVR architectures
(the `-mXXXX-emu` libraries).

NOTE - This needs a user manual, but doesn't yet have one. See the
`SynchBox` and `DigiBox` projects for nontrivial sample code in the meantime.


## Using the Libraries

To use the libraries:

* Add `#include "neuravr.h"` to the project's master header file.

* Add `#include "neurapp-oo.h"` to the project's master header file if using
the application framework.

* When compiling for an AVR target, add the following `avr-gcc` flags ahead
of your sources:
```
-I(path to /include) -L(path to /lib)
-Os -fno-exceptions
-D__AVR_ATmegaXXXX__ -mmcu=atmegaXXXX
```
...and the following `avr-gcc` flags after your sources:

`-lneurapp-mXXXX -lneur-mXXXX`

* When compiling for an emulated target, add the following `g++` flags ahead
of your sources:
```
-I(path to /include) -L(path to /lib)
-O2 -std=c++11 -pthread
-D__AVR_ATmegaXXXX__
```
...and the following `g++` flags after your sources:

`-lneurapp-mXXXX-emu -lneur-mXXXX-emu`


## Folders

Folders with files that you need in order to use the firmware:

* `include` - Headers.
* `lib` - Library binaries.

Folders with files that you can read to help with using the firmware:

* `testing` - Test code (small stand-alone applications).

Folders with files that you might want to read before modifying the firmware:

* `datasheets` - Vendor-supplied datasheets.
* `notes` - Various notes that are useful if you're planning to modify the
firmware code.

Folders with the firmware code itself:

* `core` - Code that's hardware-independent.
* `emulation` - Hardware-specific code for compiling for use on workstations
(to test applications in an environment that has a debugger).
* `m2560` - Hardware-specific code for the ATmega2560, used in the Arduino
Mega 2560 board.
* `m328p` - Hardware-specific code for the ATmega328P, used in the Arduino
Uno board.
* `skeleton-oo` - Implementation of the application framework.


## Rebuilding the Libraries

To rebuild everything:

./do-rebuild.sh

This removes and replaces everything in `include` and `lib`.

The project files are in `core` and `mXXXX`. Only edit them if you're
intimately familiar with bare-metal AVR programming and have the datasheets
handy.


_(This is the end of the file.)_
