Nebula
======

Nebula is an emulator for the DCPU-16 processor.

Nebula supports the full specification of the DCPU-16 based on [version 1.7 of the specification](http://dcpu.com/dcpu-16/). Nebula also supports the following hardware peripherals:

* The generic clock.
* The LEM1802 monitor.
* The generic keyboard.
* The M35FD floppy drive
  * Press `F10` to insert a write-protected floppy disk.
  * Press `F11` to insert a normal floppy disk.
  * Press `F12` to eject the disk.

## Use and Supported Platforms

Nebula is written to the C++11 standard, which is only supported by
the latest versions of the GCC and Clang compilers at the time of this
writing.

Aside from incompatibilities with modern C++11 language and library
features, Nebula should be entirely platform independent. This is
accomplished by limiting dependencies to cross-platform libraries including:

* Boost (>= 1.54)
* SDL (== 1.2)
* Google-Test (bundled)

To build Nebula, CMake (>= 2.8) is required.

Nebula is released under the terms of [version 2.0 of the
Apache License](http://www.apache.org/licenses/LICENSE-2.0).

## Implementation Notes

Nebula uses the concurrency features of C++11 to simulate each
peripheral device. Each peripheral is therefore able to operate
independently at its own clock frequency. The peripherals communicate
with the processor asynchronously via interrupts only.

