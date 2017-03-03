// Copyright 2014 Technical Machine, Inc. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

#include "common/util.h"
#include "samd/usb_samd.h"

#include <string.h>
#include <stdbool.h>

#include "boot.h"
#include "common/nvm.h"

__attribute__ ((section(".copyright")))
__attribute__ ((used))
const char copyright_note[] = COPYRIGHT_NOTE;

volatile bool exit_and_jump = 0;

// set at runtime
uint32_t total_flash_size;

// Board customization. You may define these functions in another file
// and that new code will replace the stubs defined here.
void board_setup_early(void) __attribute__((weak, alias("noopFunction")));
void board_setup_late(void) __attribute__((weak, alias("noopFunction")));
void board_reset_cleanup(void) __attribute__((weak, alias("noopFunction")));
bool button_pressed(void) __attribute__((weak));


/*** SysTick ***/

volatile uint32_t g_msTicks;

/* SysTick IRQ handler */
void SysTick_Handler(void) {
	g_msTicks++;
}

void delay_ms(unsigned ms) {
	unsigned start = g_msTicks;
	while (g_msTicks - start <= ms) {
		__WFI();
	}
}

void init_systick() {
	if (SysTick_Config(48000000 / 1000)) {	/* Setup SysTick Timer for 1 msec interrupts  */
		while (1) {}								/* Capture error */
	}
	NVIC_SetPriority(SysTick_IRQn, 0x0);
	g_msTicks = 0;
}

/*** USB / DFU ***/

void dfu_cb_dnload_block(uint16_t block_num, uint16_t len) {
	if (usb_setup.wLength > DFU_TRANSFER_SIZE) {
		dfu_error(DFU_STATUS_errUNKNOWN);
		return;
	}

	if (block_num * DFU_TRANSFER_SIZE > FLASH_FW_SIZE) {
		dfu_error(DFU_STATUS_errADDRESS);
		return;
	}

	nvm_erase_row(FLASH_FW_START + block_num * DFU_TRANSFER_SIZE);
}

void dfu_cb_dnload_packet_completed(uint16_t block_num, uint16_t offset, uint8_t* data, uint16_t length) {
	unsigned addr = FLASH_FW_START + block_num * DFU_TRANSFER_SIZE + offset;
	nvm_write_page(addr, data, length);
}

unsigned dfu_cb_dnload_block_completed(uint16_t block_num, uint16_t length) {
	return 0;
}

void dfu_cb_manifest(void) {
	exit_and_jump = 1;
}

void noopFunction(void)
{
	// Placeholder function for code that isn't needed. Keep empty!
}

static void hardware_detect(void)
{
	// what kind of chip are we installed on?
	// .. don't care

	// how big is the flash tho
	uint16_t page_size = 1 << (NVMCTRL->PARAM.bit.PSZ + 3);

	total_flash_size = NVMCTRL->PARAM.bit.NVMP * page_size;
}

void bootloader_main()
{
	// Hook here for very early hardware init that some boards need
	board_setup_early();

	hardware_detect();

	clock_init_usb(GCLK_SYSTEM);
	init_systick();
	nvm_init();

	// Hook here for "early" hardware init that some boards need
	board_setup_late();

	__enable_irq();

	pin_mux(PIN_USB_DM);
	pin_mux(PIN_USB_DP);
	usb_init();
	usb_attach();

	while(!exit_and_jump) {
		__WFI(); /* conserve power */
	}

	delay_ms(25);

	usb_detach();
	nvm_invalidate_cache();

	delay_ms(100);

	// Hook: undo any special setup that board_setup_late might be needed to
	// undo the setup the bootloader code has done.
	board_reset_cleanup();

#ifdef USE_CORE_RESET
	jump_to_flash(FLASH_FW_ADDR, 0);
#elif
	NVIC_SystemReset();
#endif
}

bool flash_valid() {
	unsigned sp = ((unsigned *)FLASH_FW_ADDR)[0];
	unsigned ip = ((unsigned *)FLASH_FW_ADDR)[1];

	return     sp > 0x20000000
			&& ip >= 0x00001000
			&& ip <  0x00400000;
}


bool button_pressed(void)
{
	// Repalce this function (in another file) if you want to 
	// test for a pin or button being pressed during boot time
	// to get into DFU mode.
	return false;
}

bool bootloader_sw_triggered()
{
	// Was reset caused by watchdog timer (WDT)?
	return PM->RCAUSE.reg & PM_RCAUSE_WDT;
}

int main_bl() {
	if (!flash_valid() || button_pressed() || bootloader_sw_triggered()) {
		bootloader_main();
	}

	jump_to_flash(FLASH_FW_ADDR, 0);
	return 0;
}
