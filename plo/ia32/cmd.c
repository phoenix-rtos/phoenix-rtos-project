/*
 * Phoenix-RTOS
 *
 * plo - perating system loader
 *
 * Loader commands
 *
 * Copyright 2012,2017 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Pawel Kolodziej
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "config.h"
#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "msg.h"
#include "phfs.h"
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
void cmd_lspci(char *s);


struct {
	void (*f)(char *);
	char *cmd;
	char *help;
} cmds[] = {
	{ cmd_dump,    "dump", "    - dumps memory, usage: dump <segment>:<offset>" },
	{ cmd_go,      "go!", "     - starts Phoenix-RTOS loaded into memory" },
	{ cmd_help,    "help", "    - prints this message" },
	{ cmd_load,    "load", "    - loads Phoenix-RTOS, usage: load [<boot device>]" },
	{ cmd_memmap,  "mem", "     - prints physical memory map" },
	{ cmd_cmd,     "cmd", "     - boot command, usage: cmd [<command>]" },
	{ cmd_timeout, "timeout", " - boot timeout, usage: timeout [<timeout>]" },
	{ cmd_save,    "save", "    - saves configuration" },
	{ cmd_lspci,   "lspci", "   - enumerates PCI buses" },
	{ NULL, NULL, NULL }
};


struct {
	char *name;
	unsigned int pdn;
} devices[] = {
	{ "com1", PDN_COM1 },
	{ "com2", PDN_COM2 },
	{ "floppy", PDN_FLOPPY },
	{ "hd0", PDN_HD0 },
	{ "hd1", PDN_HD1 },
	{ "hd2", PDN_HD2 },
	{ "hd3", PDN_HD3 },
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


void cmd_dump(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0;
	int xsize = 16;
	int ysize = 16;
	unsigned int x, y;
	u16 segm, offs, isegm, ioffs;
	u8 b;

	/* Get address */
	cmd_skipblanks(s, &p, DEFAULT_BLANKS);

	if (cmd_getnext(s, &p, ":", NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad segment!\n");
		return;
	}
	segm = plostd_ahtoi(word);

	/* Skip colon */
	cmd_skipblanks(s, &p, DEFAULT_BLANKS ":");

	if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
		plostd_printf(ATTR_ERROR, "\nSize error!\n");
		return;
	}
	if (*word == 0) {
		plostd_printf(ATTR_ERROR, "\nBad offset!\n");
		return;
	}
	offs = plostd_ahtoi(word);

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "Memory dump from %p:%p\n", segm, offs);
	plostd_printf(ATTR_LOADER, "--------------------------\n");

	for (y = 0; y < ysize; y++) {
		plostd_printf(ATTR_LOADER, "%p:%p   ", segm, offs);
		isegm = segm;
		ioffs = offs;

		/* Print byte values */
		for (x = 0; x < xsize; x++) {
			low_copyfrom(segm, offs, &b, 1);
			if (b & 0xf0)
				plostd_printf(ATTR_LOADER, "%x ", b);
			else
				plostd_printf(ATTR_LOADER, "0%x ", b);

			if (offs++ == 0xffff)
				segm++;
		}
		plostd_printf(ATTR_LOADER, "  ");

		/* Print ASCII representation */
		segm = isegm;
		offs = ioffs;

		for (x = 0; x < xsize; x++) {
			low_copyfrom(segm, offs, &b, 1);
			if ((b <= 32) || (b > 127))
				plostd_printf(ATTR_LOADER, ".", b);
			else
				plostd_printf(ATTR_LOADER, "%c", b);

			if (offs++ == 0xffff)
				segm++;
		}
		plostd_printf(ATTR_LOADER, "\n");
	}
	return;
}


void cmd_go(char *s)
{
	plostd_printf(ATTR_LOADER, "\nStarting Phoenix-RTOS...\n");
	low_launch();
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


void cmd_loadkernel(unsigned int pdn, char *arg, u16 *po)
{
	char *path;
	s32 h;
	u32 p, loffs, hdrsz;
	u16 hdrszo;
	Elf32_Ehdr hdr;
	Elf32_Phdr phdr;
	Elf32_Word i, k;
	Elf32_Word size, l;
	u8 buff[384];
	int err;
	u32 minaddr = 0xffffffff, maxaddr = 0, start = KERNEL_BASE;

	path = arg != NULL ? arg : KERNEL_PATH;
	if ((h = phfs_open(pdn, path, 0)) < 0) {
		plostd_printf(ATTR_ERROR, "Kernel not found!\n");
		return;
	}

	if (arg != NULL) {
		h = plostd_ahtoi(arg);
		minaddr = low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL) | ((u32)low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 2) << 16);
		maxaddr = minaddr + (low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE) | ((u32)low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 2) << 16));
		start = ((maxaddr + 0xfff) & (u32)0xfffff000) + 0x1000;
		maxaddr = start;
	}

	/* Read ELF header */
	p = 0;
	if (phfs_read(pdn, h, &p, (u8 *)&hdr, (u32)sizeof(Elf32_Ehdr)) < 0) {
		plostd_printf(ATTR_ERROR, "Can't read ELF header!\n");
		return;
	}
	if ((hdr.e_ident[0] != 0x7f) && (hdr.e_ident[1] != 'E') && (hdr.e_ident[2] != 'L') && (hdr.e_ident[3] != 'F')) {
		plostd_printf(ATTR_ERROR, "File isn't ELF object!\n");
		return;
	}

	low_setfar(SYSPAGE_SEG, *po, (u16)(hdr.e_entry & 0xffff));
	low_setfar(SYSPAGE_SEG, *po + 2, (u16)(hdr.e_entry >> 16));
	(*po) += 4;

	hdrsz = 0;
	hdrszo = (*po);
	(*po) += 4;

	/* Read program segments */
	for (k = 0; k < hdr.e_phnum; k++) {
		p = hdr.e_phoff + k * sizeof(Elf32_Phdr);
		if (phfs_read(pdn, h, &p, (u8 *)&phdr, (u32)sizeof(Elf32_Phdr)) < 0) {
			plostd_printf(ATTR_ERROR, "Can't read Elf32_Phdr, k=%d!\n", k);
			return;
		}

		if ((phdr.p_type == PT_LOAD) && (phdr.p_vaddr != 0)) {

			hdrsz++;

			if (start == maxaddr)
				start = (phdr.p_vaddr & ~0xfff) - maxaddr;

			/* Calculate kernel memory parameters */
			if (minaddr > phdr.p_vaddr - start)
				minaddr = phdr.p_vaddr - start;
			if (maxaddr < phdr.p_vaddr - start + phdr.p_memsz)
				maxaddr = phdr.p_vaddr - start + phdr.p_memsz;

			loffs = phdr.p_vaddr - start;

			/* Update segment addr, size, flags */
			low_setfar(SYSPAGE_SEG, *po, (u16)(loffs & 0xffff));
			low_setfar(SYSPAGE_SEG, *po + 2, (u16)(loffs >> 16));
			*po += 4;
			low_setfar(SYSPAGE_SEG, *po, (u16)(phdr.p_memsz & 0xffff));
			low_setfar(SYSPAGE_SEG, *po + 2, (u16)(phdr.p_memsz >> 16));
			*po += 4;
			low_setfar(SYSPAGE_SEG, *po, (u16)(phdr.p_flags & 0xffff));
			low_setfar(SYSPAGE_SEG, *po + 2, (u16)(phdr.p_flags >> 16));
			*po += 4;
			low_setfar(SYSPAGE_SEG, *po, (u16)(phdr.p_vaddr & 0xffff));
			low_setfar(SYSPAGE_SEG, (*po) + 2, (u16)(phdr.p_vaddr >> 16));
			*po += 4;
			low_setfar(SYSPAGE_SEG, *po, (u16)(phdr.p_filesz & 0xffff));
			low_setfar(SYSPAGE_SEG, (*po) + 2, (u16)(phdr.p_filesz >> 16));
			*po += 4;
			low_setfar(SYSPAGE_SEG, *po, (u16)(phdr.p_align & 0xffff));
			low_setfar(SYSPAGE_SEG, (*po) + 2, (u16)(phdr.p_align >> 16));
			*po += 4;

			plostd_printf(ATTR_LOADER, "Reading segment %p%p at %p%p:  ",
				(u16)(phdr.p_vaddr >> 16), (u16)(phdr.p_vaddr & 0xffff), (u16)(loffs >> 16), (u16)(loffs & 0xffff));

			for (i = 0; i < phdr.p_filesz / sizeof(buff); i++) {

				p = phdr.p_offset + i * sizeof(buff);
				if ((err = phfs_read(pdn, h, &p, buff, (u32)sizeof(buff))) < 0) {
					plostd_printf(ATTR_ERROR, "\nCan't read segment data, k=%d!\n", k);
					return;
				}

				low_copytoabs(loffs, buff, sizeof(buff));
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

			p = phdr.p_offset + i * sizeof(buff) + size;

			low_copytoabs(loffs, buff, size);
			loffs += size;
			cmd_progress(i);

			/* zero uninitialized memory */
			for (i = 0; i < sizeof(buff); i++)
				buff[i] = 0;

			for (i = 0; i < phdr.p_memsz - phdr.p_filesz;) {
				int len = min(phdr.p_memsz - phdr.p_filesz - i, sizeof(buff));

				low_copytoabs(loffs, buff, len);
				loffs += len;
				i += len;
			}
			plostd_printf(ATTR_LOADER, "%c[ok]\n", 8);
		}
	}

	/* Update hdrsz */
	low_setfar(SYSPAGE_SEG, hdrszo, (u16)(hdrsz & 0xffff));
	low_setfar(SYSPAGE_SEG, hdrszo + 2, (u16)(hdrsz >> 16));

/* plostd_printf(ATTR_LOADER, "maxaddr %p%p\n", (u16)(maxaddr >> 16), (u16)(maxaddr & 0xffff)); */

	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 0, (u16)(minaddr & 0xffff));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 2, (u16)(minaddr >> 16));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 0, (u16)((maxaddr - minaddr) & 0xffff));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 2, (u16)((maxaddr - minaddr) >> 16));

	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_CONSOLE + 0, 0);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_CONSOLE + 2, 0);

	return;
}


void cmd_load(char *s)
{
	char word[LINESZ + 1];
	unsigned int p = 0, dn;
	u16 i, po;
	u32 kernel, kernelsize;

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

	/* Load kernel */
	plostd_printf(ATTR_LOADER, "\nLoading kernel\n");

	po = SYSPAGE_OFFS_PROGS;
	cmd_loadkernel(devices[dn].pdn, NULL, &po);

	/* Store kernel size */
	kernel = low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL) | ((u32)low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 2) << 16);
	kernelsize = low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE) | ((u32)low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 2) << 16);

	po = SYSPAGE_OFFS_PROGS;

	/* Load programs */
	for (i = 0;; i++) {
		cmd_skipblanks(s, &p, DEFAULT_BLANKS);
		if (cmd_getnext(s, &p, DEFAULT_BLANKS, NULL, word, sizeof(word)) == NULL) {
			plostd_printf(ATTR_ERROR, "\nSize error!\n");
			return;
		}

		if (*word == 0)
			break;

		plostd_printf(ATTR_LOADER, "\nLoading program (offs=%s)\n", word);
		cmd_loadkernel(devices[dn].pdn, word, &po);
	}

	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_PROGSSZ, i);

	/* Update kernel size */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 0, (u16)(kernel & 0xffff));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 2, (u16)(kernel >> 16));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 0, (u16)(kernelsize & 0xffff));
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 2, (u16)(kernelsize >> 16));

	return;
}


void cmd_memmap(char *s)
{
	int k, i;
	int id;
	char *stat;
	low_mmitem_t mmitem;

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "BIOS e820 memory map\n");
	plostd_printf(ATTR_LOADER, "--------------------\n");

	for (k = 0; k < MM_MAXSZ; k++) {
		if (low_mmget(k, &mmitem) == ERR_LOW_MMSIZE)
			break;

		if (mmitem.attr == 1)
			stat = "free";
		else
			stat = "reserved";
		plostd_printf(ATTR_LOADER, "%P%P  +%P%P  -  %s\n", mmitem.addr_hi, mmitem.addr_lo,
			mmitem.len_hi, mmitem.len_lo, stat);
	}

	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "plo map\n");
	plostd_printf(ATTR_LOADER, "-------\n");

	plostd_printf(ATTR_LOADER, "%p%p  +0000%p  -  GDT\n", GDT_SEG >> 12, GDT_SEG << 4, GDT_SIZE);
	plostd_printf(ATTR_LOADER, "%p%p  +0000%p  -  IDT\n", IDT_SEG >> 12, IDT_SEG << 4, IDT_SIZE);
	plostd_printf(ATTR_LOADER, "%p%p  +0000%p  -  syspage\n", SYSPAGE_SEG >> 12, SYSPAGE_SEG << 4, 0x1000);
	plostd_printf(ATTR_LOADER, "%p%p  +0000%p  -  pdir\n", PDIR_SEG >> 12, PDIR_SEG << 4, 0x1000);
	plostd_printf(ATTR_LOADER, "%p%p  +0000%p  -  ptable\n", PTABLE_SEG >> 12, PTABLE_SEG << 4, 0x1000);
	plostd_printf(ATTR_LOADER, "0000%p  +0000%p  -  stack\n", INIT_ESP - STACK_SIZE, STACK_SIZE);

	plostd_printf(ATTR_LOADER, "0000%p  +0000%p  -  plo\n", 0x7c00, CACHE_OFFS - 0x7c00 + CACHE_SIZE * SECTOR_SIZE);

	plostd_printf(ATTR_LOADER, "%p%p  +%p%p  -  kernel\n",
	              low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL + 2), low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNEL),
	              low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE + 2), low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_KERNELSIZE));
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
	u16 err;

	plostd_printf(ATTR_LOADER, "\n");
	if (err = _plo_save())
		plostd_printf(ATTR_ERROR, "Can't save configuration [err=0x%x]!\n", err);
	else
		plostd_printf(ATTR_LOADER, "Configuration saved\n");

	return;
}


#define PCI_CREG(b, d, f, r) (0x80000000 | ( (u32)b << 16 ) | ((u32)d << 11) | ((u32)f << 8) | (r << 2))


/* Function lists devices connected to local PCI buses */
void cmd_lspci(char *s)
{
	u16 err;
	unsigned int b, d, f, lf;
	u32 dv, base;
	u16 cl;
	u8 irq;
	u8 type;
	plostd_printf(ATTR_LOADER, "\n");
	plostd_printf(ATTR_LOADER, "PCI devices:\n");
	plostd_printf(ATTR_LOADER, "------------\n");

	for (b = 0; b < 256; b++) {
		for (d = 0; d < 32; d++) {
			for (f = 0; f < 8; f++) {
				low_outd(0xcf8, PCI_CREG(b, d, f, 0));
				dv = low_ind(0xcfc);
				if (dv == 0xffffffff)
					continue;

				low_outd(0xcf8, PCI_CREG(b, d, f, 0));
				dv = low_ind(0xcfc);

				low_outd(0xcf8, PCI_CREG(b, d, f, 2));
				cl = (u16)(low_ind(0xcfc) >> 16);

				low_outd(0xcf8, PCI_CREG(b, d, f, 4));
				base = low_ind(0xcfc);

				low_outd(0xcf8, PCI_CREG(b, d, f, 3));
				type = (u8)(low_ind(0xcfc) >> 16 & 0xff);

				low_outd(0xcf8, PCI_CREG(b, d, f, 15));
				irq = (u8)(low_ind(0xcfc) & 0xff);

				plostd_printf(ATTR_LOADER, "%p:%p.%x, vd=%p:%p, class=%p, base=%p%p, irq=%d mf=%s\n",
					b, d, f, dv, cl, (u16)(base >> 16), (u16)(base & 0xffff), irq, (type & 0x80 ?"true":"false"));

				if (((type & 0x80) == 0) && f == 0)
					break; /* not a multifunction device */
			}
		}
	}
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
