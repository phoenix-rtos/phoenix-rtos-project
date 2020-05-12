/**
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * LD script for Vybrid6 booting and running from on-chip RAM
 *
 * Copyright 2014 Phoenix Systems
 * Author: Jacek Popko, Katarzyna Baranowska
 *
 * This file is part of Phoenix-RTOS.
 * 
 * %LICENSE%
 */

OUTPUT_FORMAT("elf32-littlearm")

ENTRY(_start)

MEMORY
{
	/* regular on-chip ram */
	ocram (rwx)     :	ORIGIN = 0x3f000000, LENGTH = 512K
	/* gfx on-chip ram */
	gfxram (rwx)    :	ORIGIN = 0x3f400000, LENGTH = 512K
}

REGION_ALIAS("ram", ocram);
REGION_ALIAS("ram2", gfxram);

STACK_START = ORIGIN(ocram) + LENGTH(ocram) - 8;
_ram_begin = ORIGIN(ram);

SECTIONS
{
	.boot :
	{
		_plo_img_begin = .;
		*(.bootrom_data)
		. = ALIGN(0x800);
	}

	/* Code sections */
	.text	:
	{
		_ftext = .;
		_ftext_rom = LOADADDR(.text);
		*(.text .stub .text.* .gnu.linkonce.t.*)
		*(.gnu.warning)
	} = 0

	.init			: { KEEP (*(.init)) }	= 0
	.fini			: { KEEP (*(.fini)) }	= 0

	/* Exception handlers */
	.eh_frame_hdr	: { *(.eh_frame_hdr) }
	.eh_frame		: { KEEP (*(.eh_frame)) }
	.gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) }

	/* Constructors and destructors */
	.ctors          :
	{
		KEEP (*crtbegin*.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	}

	.dtors          :
	{
		KEEP (*crtbegin*.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	}
	.jcr            : { KEEP (*(.jcr)) }

	/* Read only data */
	.rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
	.rodata1        : { *(.rodata1) }

	_etext = .;
	PROVIDE (etext = .);

	. = ALIGN(4);

	/* Data */
	.data           :
	{
		_fdata = .;
		_fdata_rom = LOADADDR(.data);
		*(.data .data.* .gnu.linkonce.d.*)
		*(.data1)
		SORT(CONSTRUCTORS)
	} = 0

	.got :  { *(.got.plt) *(.got) }
	.sdata : { *(.sdata .sdata.* .gnu.linkonce.s.*) }
	.lit4 : { *(.lit4) }
	.lit8 : { *(.lit8) }
	.lita : { *(.lita) }
	_edata = .;
	PROVIDE (edata = .);

	_plo_img_end = .;
	
	/* BSS */
    .bss           :
	{
		_fbss = .;
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)	
		. = ALIGN(2);
		_ebss = .;
		_fheap = .;
	} = 0
	
	_fbss_phy = LOADADDR(.bss);
	_ebss_phy = LOADADDR(.bss) + SIZEOF(.bss);
	_plo_img_size = _plo_img_end - _plo_img_begin;
	
	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}
