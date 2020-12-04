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



## Rebuilding the Libraries


To rebuild everything:

./do-rebuild.sh

This removes and replaces everything in "include" and "lib".

The project files are in "core" and "mXXXX". Only edit them if you're
intimately familiar with bare-metal AVR programming and have the datasheets
handy.


_This is the end of the file._
