# Attention Circuits Control Laboratory - Atmel AVR firmware
# Change log, bug list, and feature request list.
Written by Christopher Thomas.


## Bugs and feature requests:

* Add base-10 integer fast-printing support and remove snprintf().

* Add mutex locks for "every tick" and "high priority poll" operations in the
application framework. The idea is to be able to guarantee atomic operation
without disabling interrupts entirely.

* Add a benchmarking debug function that spins the high priority polling
loop N times and times it.

* Add support for multiple UARTs.

* Make a manual describing how to use these libraries.

* Add a section describing how to properly daisy-chain command handling and
add multiple command lists for child classes of event handlers.

* Add a section describing what hooks are needed for a new AVR architecture.


## Low-priority feature requests:

* Add support for the 32u4 architecture.

* Move Arduino IDE shim code here, from USE SyncBox.

* Make ready-made event handlers for common app operations.


## History (most recent changes first):

* 29 Jul 2020 -- Added hex fast-printing support.

* 15 Jun 2020 -- Added digital GPIO support.

* 13 Jun 2020 -- Added ADC support. Added stubs for GPIO support.

* 29 May 2020 -- Added "high priority polling" callbacks.

* 25 May 2020 -- Added mem free and tick overrun debug commands/tracking.

* 20 May 2020 -- Added a deeper buffer for report message buffering (app).

* 29 Apr 2020 --
Added an "emulation" build target for testing on workstations.

* 23 Apr 2020 --
Added duplicate checking in the handler list to allow daisy-chaining of
parent and child class commands for a child handler.

* 18 Mar 2020 -- Fixed end-of-list check for command/opcode list iteration.

* 18 Mar 2020 -- Added event-handler help screens.

* 16 Mar 2020 -- Fixed message buffer issues in app framework reporting loop.

* 03 Mar 2020 -- Changed license to CC-by (commercial use is allowed).

* 12 Dec 2018 -- Added CC-by-nc license.


_This is the end of the file._
