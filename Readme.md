
# DAFU Boot
## DFU Bootloader for Atmel ARM parts 
_for at least the Atmel SAM D21 and similar_

Based on [t2-firmware/boot/](https://github.com/tessel/t2-firmware)

Compatible with [DFU Utils](http://dfu-util.sourceforge.net/)

## Background

This is 4k of binary code that will turn a typical D21 board into a DFU target.
It makes as few assumptions as possible about the other hardware on the board, 
but it could be customized if needed.

### Project Goals

- generalized so can run on any D21 (and maybe others) ideally without pre-configuration
- make it easy to rebrand and/or be generic
- minimalist and self-contained Makefile
- make it easy to link into other projects as a linkable object
- compile-time option for a secure mode that blocks any reads
- support pre and post boot customization options
- stay under 4k and open-source


## Installation

Requirements:

- GCC for ARM toolchain. This is expected to be installed with prefix "arm-none-eabi-"
- gnu make

Steps:

Get the submodules. This will populate `deps/sam0` and `deps/usb`

	git submodules init
	git submodules update

To build, run make:

	make

Main binaries will be in these files:

	dafu.elf
	dafu.bin

Other useful files:

	dafu.lss - Detailed listing
	dafu.hex - Intel-format hex file
	dafu.o - See next section

## Usage

If there seems to be a valid program in flash at 0x1000, this code will run it. That's 
the normal boot sequence in the field. If your mainline code resets because of the watchdog
timeout, the DAFU code will start instead. Once DAFU is running, then you should see
a USB device enumerated with the PID/VID values from the Makefile.

Install your code via this command:

	dfu-util -aFlash -D foo.bin

Your binary should start at memory address 0x1000 (4k) and the first few words
should be the reset vector that points to your mainline code. In other words, if
DAFU is normally pre-pended to your code, you should remove it before uploading with
DFU util.

## Linked Object File (dafu.o)

You can include the `dafu.o` object file into your main project Makefile.

If you're using the standard linker scripts that Atmel provides,
and you list this object file before others in the link step, it
should end up positioned at offset zero. Because it's 4k (0x1000)
in size, your normal exception table (interrupt vector table) will
start at 0x1000 which is where it needs to be. The linker will
adjust your code to be at 0x4000.

This relies on a section called ".vectors" being hardcoded to the
start of flash memory. 

You can also just load the DAFU binary at offset zero (by any means),
and link your main program to load and run at 0x1000.

# Todo List

- check that DFU cannot write to "NVM User Row"


