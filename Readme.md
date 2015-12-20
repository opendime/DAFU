
# DAFU Boot
## DFU Bootloader for Atmel ARM parts 
_at least the Atmel SAM D21 and similar_

Originally based on <https://github.com/tessel/t2-firmware>

## Background

This is 4k of binary code that will turn a typical D21 board into a DFU target.
It makes as few assumptions as possible about the other hardware on the board, 
but it could be customized if needed.

## Installation

Requirements:
- GCC for ARM toolchain. This is expected to be installed with prefix "arm-none-eabi-"
- gnu make

Steps:

- Get the submodules (this will populate deps/sam0 and deps/usb):

	git submodules init
	git submodules update

- To build, run make

	make

- Main binaries will be in these files:

	dafu.elf
	dafu.bin

Other useful files:

	dafu.lss - Detailed listing
	dafu.hex - Intel-format hex file
	dafu.o - See next section

## Linked object

You can include the "dafu.o" object file into your main project.
If you're using the standard linker scripts that Atmel provides,
and you list this object file before others in the link step, it
should end up positioned at offset zero (and it needs to). Because
it's 4k (0x1000) is size, your normal exception table (interrupt
vector table) will start at 0x1000 which is where it needs to be.

This relies on a section called ".vectors" being hardcoded to the
start of flash memory.

You can also just load the binary at offset zero, and link your main
program at 0x1000.

