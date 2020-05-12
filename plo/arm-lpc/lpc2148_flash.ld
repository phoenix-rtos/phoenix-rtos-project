/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Linker script for Philips LPC2138 ARM microcontroller
 * applications that execute from Flash .
 *
 * Copyright 2006 Radoslaw F. Wawrzusiak
 *
 * Copyright 2006 D.W. Hawkins (dwh@ovro.caltech.edu)
 * Derived from: Philips 05: Project Number AR1803
 *
 * This file is part of Phoenix-RTOS.
 *
 * Phoenix-RTOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Phoenix-RTOS kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phoenix-RTOS kernel; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* The LPC2138 has 512kB of Flash , and 32kB SRAM */
MEMORY
{
	flash (rx)	: org = 0x00000000 , len = 0x00080000
	sram (rw)	: org = 0x40000000 , len = 0x00008000
}

SECTIONS
{
	/*
	 * --------------------------------------------------------------------
	 * . text section (executable code)
	 * --------------------------------------------------------------------
	 */
	.text :
	{
		_startup.o (.text)
		*(.text)
		/* *(.glue_7t) *(.glue_7) */
	} > flash
	. = ALIGN(4);

	/*
	 * --------------------------------------------------------------------
	 * .rodata section (read/only (const) initialized variables)
	 * --------------------------------------------------------------------
	 */
	.rodata :
	{
		*(.rodata)
	} > flash
	. = ALIGN(4);

	/* End-of-text symbols */
	_etext = . ;
	PROVIDE (etext = .);

	/* --------------------------------------------------------------------
	 * . data section (read/write initialized variables )
	 * --------------------------------------------------------------------
	 *
	 * The values of the initialized variables are stored
	 * in Flash , and the startup code copies them to SRAM.
	 *
	 * The variables are stored in Flash starting at _etext,
	 * and are copied to SRAM address _data to _edata.
	 */
	.data : AT (_etext)
	{
		_data = . ;
		*(.data)
	} > sram
	. = ALIGN(4);

	_edata = . ;
	PROVIDE (edata = .);

	/*
	 * --------------------------------------------------------------------
	 * .bss section (uninitialized variables)
	 * --------------------------------------------------------------------
	 *
	 * These symbols define the range of addresses in SRAM that
	 * need to be zeroed.
	 */
	.bss :
	{
		_bss = . ;
		*(.bss)
		*(COMMON)
	} > sram
	. = ALIGN(4);
	_ebss = . ;

	_end = . ;
	PROVIDE (end = .);

	/* Stabs debugging sections. */
	.stab			0 : { *(.stab) }
	.stabstr		0 : { *(.stabstr) }
	.stab.excl		0 : { *(.stab.excl) }
	.stab.exclstr	0 : { *(.stab.exclstr) }
	.stab.index		0 : { *(.stab.index) }
	.stab.indexstr	0 : { *(.stab.indexstr) }
	.comment		0 : { *(.comment) }
	/* DWARF debug sections.
	 * Symbols in the DWARF debugging sections are relative to the beginning
	 * of the section so we begin them at 0.
	 */
	/* DWARF 1 */
	.debug			0 : { *(.debug) }
	.line			0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }
	/* DWARF 1 . 1 and DWARF 2 */
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info		0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line		0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str		0 : { *(.debug_str) }
	.debug_loc		0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}