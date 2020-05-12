/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Loader commands
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Copyright 2006 Radoslaw F. Wawrzusiak
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

#include "config.h"
#include "errors.h"
#include "types.h"
#include "low.h"
#include "plostd.h"
#include "iap.h"
#include "elf.h"


#define DEFAULT_BLANKS  " \t"
#define DEFAULT_CITES   "\""


void cmd_dump(char *s);
void cmd_go(char *s);
void cmd_help(char *s);
void cmd_load(char *s);
void cmd_memmap(char *s);
void cmd_cmd(char *s);
void cmd_timeout(char *s);
void cmd_save(char *s);
void cmd_date(char *s);
void cmd_info(char *s);
void cmd_get(char *s);
void cmd_set(char *s);


const struct {
	void (*f)(char *);
	char *cmd;
	char *help;
} cmds[] = {
	{ cmd_dump,		"dump", "    - dumps memory, usage: dump <addr>" },
	{ cmd_go,		"go!", "     - starts Phoenix-RTOS loaded into memory" },
	{ cmd_help,		"help", "    - prints this message" },
	{ cmd_load,		"load", "    - loads Phoenix-RTOS, usage: load [<boot device>]" },
	{ cmd_memmap,	"mem", "     - prints physical memory map" },
	{ cmd_cmd,		"cmd", "     - boot command, usage: cmd [<command>]" },
	{ cmd_timeout,	"timeout", " - boot timeout, usage: timeout [<timeout>]" },
	{ cmd_save,		"save", "    - saves configuration" },
	{ cmd_date,		"date", "    - view/set date/time" },
	{ cmd_info,		"info", "	 - device info (partId, register content, status" },
	{ cmd_get,		"get", "	 - get data at address, usage: get <addr>" },
	{ cmd_set,		"set", "	 - set data at address, usage: set <addr> <val>" },
	{ NULL, NULL, NULL }
};


const struct {
	char *name;
	unsigned int pdn;
} devices[] = {
	{ "com0", 0 },
	{ "com1", 1 },
	{ NULL, NULL }
};


/* Function prints progress indicator */
void cmd_progress(u32 p)
{
	char *states = "-\\|/";

	plostd_printf(ATTR_LOADER, "%c%c", 8, states[p % plostd_strlen(states)]);
	return;
}


/* Function skips blank characters */
void cmd_skipblanks(char *line, unsigned int *pos, char *blanks)
{
	char c, blfl;
	unsigned int i;

	while ((c = *((char *)(line + *pos))) != 0) {
		blfl = 0;
		for (i = 0; i < plostd_strlen(blanks); i++) {
			if (c == *(char *)(blanks + i)) {
				blfl = 1;
				break;
			}
		}
		if (!blfl)
			break;
		(*pos)++;
	}
	return;
}


/* Function retrieves next symbol from line */
char *cmd_getnext(char *line, unsigned int *pos, char *blanks, char *cites, char *word, unsigned int len)
{
	char citefl = 0, c;
	unsigned int i, wp = 0;

	/* Skip leading blank characters */
	cmd_skipblanks(line, pos, blanks);

	wp = 0;
	while ((c = *(char *)(line + *pos)) != 0) {

		/* Test cite characters */
		if (cites) {
			for (i = 0; cites[i]; i++) {
				if (c != cites[i])
					continue;
				citefl ^= 1;
				break;
			}

			/* Go to next iteration if cite character found */
			if (cites[i]) {
				(*pos)++;
				continue;
			}
		}

		/* Test separators */
		for (i = 0; blanks[i]; i++) {
			if (c != blanks[i])
				continue;
			break;
		}
		if (!citefl && blanks[i])
			break;

		word[wp++] = c;
		if (wp == len)
			return NULL;

		(*pos)++;
	}

	if (citefl)
		return NULL;

	word[wp] = 0;
	return word;
}


void cmd_get(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;
	u32	*addr;

	cmd_skipblanks(s, &p, DEFAULT_BLANKS);
	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad offset!\n");
		return;
	}
	addr = (u32 *)plostd_ahtoi(word);
	plostd_printf(ATTR_LOADER, "\n%p: %p\n", addr, *addr);
}


void cmd_set(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;
	u32	*addr;
	u32 val;

	cmd_skipblanks(s, &p, DEFAULT_BLANKS);
	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad addr!\n");
		return;
	}
	addr = (u32 *)plostd_ahtoi(word);

	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad value!\n");
		return;
	}
	val = plostd_ahtoi(word);
	*addr = val;
}


void cmd_dump(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;
	int xsize = 16;
	int ysize = 16;
	unsigned int x, y;
	u32 addr, iaddr;
	u8 b;

	/* Get address */
	cmd_skipblanks(s, &p, DEFAULT_BLANKS);

	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad offset!\n");
		return;
	}
	addr = plostd_ahtoi(word);

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Memory dump from %p\n", addr);
	plostd_printf(ATTR_LOADER, "--------------------------\n");

	for (y = 0; y < ysize; y++) {
		plostd_printf(ATTR_LOADER, "%p   ", addr);
		iaddr = addr;

		/* Print byte values */
		for (x = 0; x < xsize; x++) {
			b = *((u8 *)addr);
			if (b & 0xf0)
				plostd_printf(ATTR_LOADER, "%x ", b);
			else
				plostd_printf(ATTR_LOADER, "0%x ", b);

			addr++;
		}
		plostd_printf(ATTR_LOADER, "  ");

		/* Print ASCII representation */
		addr = iaddr;

		for (x = 0; x < xsize; x++) {
			b = *((u8 *)addr);
			if ((b <= 32) || (b > 127))
				plostd_printf(ATTR_LOADER, ".", b);
			else
				plostd_printf(ATTR_LOADER, "%c", b);

			addr++;
		}
		plostd_printf(ATTR_LOADER, "\n");
	}

	return;
}


void cmd_go(char *s)
{
	plostd_printf(ATTR_LOADER, "\nStarting Phoenix-RTOS...\n");

	void (*f)(void);

	f = (void*) syspage.entry;

	low_cli();
	f();
	low_sti();

	return;
}


void cmd_help(char *s)
{
	int k;

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Loader commands:\n");
	plostd_printf(ATTR_LOADER, "----------------\n");

	for (k = 0; cmds[k].cmd; k++)
		plostd_printf(ATTR_LOADER, "%s %s\n", cmds[k].cmd, cmds[k].help);
	return;
}


void cmd_loadkernel(unsigned int pdn, char *arg)
{
	char *path;
	s32 h;
	u32 p, loffs;
	Elf32_Ehdr hdr;
	Elf32_Phdr phdr;
	Elf32_Word i, k;
	Elf32_Half seg, offs;
	Elf32_Word size, l;
	u8 buff[384];
	int err;
	u32 minaddr = 0xffffffff, maxaddr = 0;

	plostd_printf(ATTR_LOADER, "\n");

	path = arg ? arg : KERNEL_PATH;
	if ((h = phfs_open(pdn, path, 0)) < 0) {
		plostd_printf(ATTR_ERROR, "Kernel not found!\n");
		return;
	}

	/* Read ELF header */
	p = 0;
	if (phfs_read(pdn, h, &p, (u8 *)&hdr, (u32)sizeof(Elf32_Ehdr)) < 0) {
		plostd_printf(ATTR_ERROR, "Can't read ELF header!\n");
		return;
	}
	if ((hdr.e_ident[0] != 0x7f) && (hdr.e_ident[1] != 'E') &&
			(hdr.e_ident[2] != 'L') && (hdr.e_ident[3] != 'F')) {
		plostd_printf(ATTR_ERROR, "Kernel file isn't ELF object!%x %c%c%c\n", hdr.e_ident[0], hdr.e_ident[1], hdr.e_ident[2], hdr.e_ident[3]);
		return;
	}

	/* Read program segments */
	for (k = 0; k < hdr.e_phnum; k++) {
		p = hdr.e_phoff + k * sizeof(Elf32_Phdr);
		if (phfs_read(pdn, h, &p, (u8 *)&phdr, (u32)sizeof(Elf32_Phdr)) < 0) {
			plostd_printf(ATTR_ERROR, "Can't read Elf32_Phdr, k=%d!\n", k);
			return;
		}

		if ((phdr.p_type == PT_LOAD) && (phdr.p_vaddr != 0)) {

			/* Calculate kernel memory parameters */
			if (minaddr > phdr.p_vaddr)
				minaddr = phdr.p_vaddr;
			if (maxaddr < phdr.p_vaddr + phdr.p_memsz)
				maxaddr = phdr.p_vaddr + phdr.p_memsz;

			plostd_printf(ATTR_LOADER, "Reading segment %p:  ", phdr.p_vaddr);

			loffs = phdr.p_vaddr;

			for (i = 0; i < phdr.p_filesz / sizeof(buff); i++) {

				p = phdr.p_offset + i * sizeof(buff);
				if ((err = phfs_read(pdn, h, &p, buff, (u32)sizeof(buff))) < 0) {
					plostd_printf(ATTR_ERROR, "\nCan't read segment data, k=%d!\n", k);
					return;
				}
				low_memcpy((void *)loffs, buff, sizeof(buff));
				loffs += sizeof(buff);
				cmd_progress(i);
			}

			/* Last segment part */
			size = phdr.p_filesz % sizeof(buff);
			if (size != 0) {
				p = phdr.p_offset + i * sizeof(buff);
				if (phfs_read(pdn, h, &p, buff, size) < 0) {
					plostd_printf(ATTR_ERROR, "\nCan't read last segment data, k=%d!\n", k);
					return;
				}
			}

			low_memcpy((void *)loffs, buff, size);
			cmd_progress(i);
			plostd_printf(ATTR_LOADER, "%c[ok]\n", 8);
		}
	}

	syspage.entry = hdr.e_entry; /* set kernel entry point */
	syspage.kernel = minaddr;
	syspage.kernelsize = maxaddr - minaddr;

	return;
}


void cmd_load(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;
	unsigned int dn;

	cmd_skipblanks(s, &p, DEFAULT_BLANKS);
	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}

	/* Show boot devices if parameter is empty */
	if (*word == 0) {
		plostd_printf(ATTR_LOADER, "\nBoot devices: ");
		for (dn = 0; devices[dn].name; dn++)
			plostd_printf(ATTR_LOADER, "%s ", devices[dn].name);
		plostd_printf(ATTR_LOADER, "\n");
		return;
	}

	for (dn = 0; devices[dn].name; dn++)  {
		if (!plostd_strcmp(word, devices[dn].name))
			break;
	}

	if (!devices[dn].name) {
		plostd_printf(ATTR_ERROR, "\n'%s' - unknown boot device!\n", word);
		return;
	}

	cmd_loadkernel(devices[dn].pdn, NULL);
	return;
}


void cmd_memmap(char *s)
{
	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Memory Map\n");
	plostd_printf(ATTR_LOADER, "--------------------\n");

	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x00, 0x40, "BOOT/FLASH/SRAM int. vectors");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x40, 0x80000-0x3040, "free FLASH memory");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x80000-0x3000, 0x3000, "Boot Block (Philips Bootloader)");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x80000, 0x40000000-0x80000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x40000000, 0x8000, "free SRAM");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x40008000, 0x7fd00000-0x40008000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x7fd00000, 0x2000, "USB DMA RAM");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x7fd02000, 0x7fffd000- 0x7fd02000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x7fffd000, 0x3000, "Boot Block (remapped from FLASH)");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0x80000000, 0x60000000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0xe0000000, 0x200000, "VPB Peripherals");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0xe0200000, 0xfe00000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0xf0000000, 0xfe00000, "RESERVED");
	plostd_printf(ATTR_LOADER, "%p  +%p  -  %s\n", 0xffe00000, 0x200000, "AHB Peripherals");


	return;
}


/* Function setups boot command */
void cmd_cmd(char *s)
{
	unsigned int p = 0;
	int l;

	plostd_printf(ATTR_LOADER, "\n");
	cmd_skipblanks(s, &p, DEFAULT_BLANKS);
	s += p;

	if (*s) {
		low_memcpy(_plo_command, s, l = min(plostd_strlen(s), CMD_SIZE - 1));
		*((char *)_plo_command + l) = 0;
	}

	plostd_printf(ATTR_LOADER, "cmd=%s\n", (char *)_plo_command);

	return;
}


/* Function setups boot timeout */
void cmd_timeout(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;

	plostd_printf(ATTR_LOADER, "\n");
	cmd_skipblanks(s, &p, DEFAULT_BLANKS);

	if (cmd_getnext(s, &p, DEFAULT_BLANKS, DEFAULT_CITES, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "Syntax error!\n");
		return;
	}
	if (*word)
		_plo_timeout = plostd_ahtoi(word);

	plostd_printf(ATTR_LOADER, "timeout=0x%x\n", _plo_timeout);
	return;
}


/* Function saves boot configuration */
void cmd_save(char *s)
{
	pdata_t pdata;
	int err = 0;

	pdata.magic = PDATA_MAGIC;
	pdata.timeout = _plo_timeout;
	low_memcpy(pdata.command, _plo_command, CMD_SIZE);

	err = iap_erase((void*) PDATA_ADDR, sizeof(pdata_t));

	if (err){
		plostd_printf(ATTR_ERROR, "\nCan't save configuration [err=0x%x]!\n", err);
		return;
	}

	err = iap_copyto((void*) PDATA_ADDR, &pdata, sizeof(pdata_t));

	if (err)
		plostd_printf(ATTR_ERROR, "\nCan't save configuration [err=0x%x]!\n", err);
	else
		plostd_printf(ATTR_LOADER, "\nConfiguration saved!\n", err);

	return;
}


/* Function lists devices connected to local PCI buses */
void cmd_date(char *s)
{

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Current date:\t%d-%d-%d\n", now->year, now->month, now->dom);
	plostd_printf(ATTR_LOADER, "Current time:\t%d:%d:%d\n", now->hour, now->min, now->sec);

	return;
}

void cmd_info(char *s)
{
	int regs[16];
	int i;
	char buff[8];

	psr_t stat;

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Part identification number: %d\n", iap_partId());
	low_getregs(regs, &stat.all);
	for(i=0; i<8; i++){
		plostd_printf(ATTR_DEBUG, "R%d\t=%p\tR%d\t=%p\n", i, regs[i], i+8, regs[i+8]);
	}
	plostd_printf(ATTR_DEBUG, "CPSR:%p\tmode:%x", stat.all, stat.mode);

	return;
}


/* Function parses loader commands */
void cmd_parse(char *line)
{
	int size, k;
	char word[LINESZ + 1], cmd[LINESZ + 1];
	unsigned int p = 0, wp;

	for (;;) {
		if (cmd_getnext(line, &p, ";", DEFAULT_CITES, word, sizeof(word)) == NULL) {
			plostd_printf(ATTR_ERROR, "\nSyntax error!\n");
			return;
		}
		if (*word == 0)
			break;

		wp = 0;
		cmd_skipblanks(word, &wp, DEFAULT_BLANKS);
		if (cmd_getnext(word, &wp, DEFAULT_BLANKS, DEFAULT_CITES, cmd, sizeof(cmd)) == NULL) {
			plostd_printf(ATTR_ERROR, "\nSyntax error!\n");
			return;
		}

		/* Find command and launch associated function */
		for (k = 0; cmds[k].cmd != NULL; k++) {
			if (!plostd_strcmp(cmd, cmds[k].cmd)) {
				cmds[k].f(word + wp);
				break;
			}
		}
		if (!cmds[k].cmd)
			plostd_printf(ATTR_ERROR, "\n'%s' - unknown command!\n", cmd);
	}

	return;
}
