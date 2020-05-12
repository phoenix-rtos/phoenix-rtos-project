/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
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
#include "phfs.h"
#include "iap.h"
#include "elf.h"
#include "bootlog.h"
#include "kernel_args.h"
#include "sign/signature.h"

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
void cmd_info(char *s);
void cmd_get(char *s);
void cmd_set(char *s);
void cmd_reset(char *s);


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
	{ cmd_info,		"info", "	 - device info (partId, register content, status" },
	{ cmd_get,		"get", "	 - get data at address, usage: get <addr>" },
	{ cmd_set,		"set", "	 - set data at address, usage: set <addr> <val>" },
	{ cmd_reset,	"reset", "	 - system reset" },
	{ NULL, NULL, NULL }
};


static const struct {
	char *name;
	unsigned int pdn;
} devices[] = {
	{ "flash", PDN_FLASH },
	{ "mtd0", PDN_FLASH },
	{ "mmcblk", PDN_MMCBLK },
	{ "mmcblk0p1", PDN_MMCBLK },
	{ "mmcblk0p2", PDN_MMCBLK },
	{ "auto", PDN_AUTO },
	{ NULL, 0 }
};


typedef struct __attribute__((packed)){
	char head;
	u16 secCyl;
} CHS_t;



typedef struct __attribute__((packed)){
	u8 status;
	CHS_t firstSector;
	u8 partitionType;
	CHS_t lastSector;
	u32 firstLBA;
	u32 sectNum;
} PartitionEntry_t;

typedef struct  __attribute__((packed)){
	union {
		char bootStrapCode[446];
		struct {
			char data[446];
		}AAP;
		struct {
			char data[446];
		}NEWLDR;
		struct {
			char data[446];
		}AST_NEC;
		struct {
			char data[446];
		}DM;
		struct {
			char data[446];
		}modern;
	} bootStrap;
	PartitionEntry_t pe[4];
	u16 signature;
} MBR_t;

typedef struct  __attribute__((packed)){
	u8 name[8];
	u64 kern_seqnum;
	u64 size;
	u64 kern_offs;
	u8 sign_type[8];
	u8 signature[480];
} KernelDesc_t;

/* Function prints progress indicator */
void cmd_progress(u32 p)
{
	char *states = "-\\|/";
	
	plostd_printf(ATTR_LOADER, "%c%c", 8, states[p % 4]);
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

	void (*f)(void);

	if (plo_syspage.ksize == 0) {
		plostd_printf(ATTR_ERROR, "\nPhoenix-RTOS size is 0.\n");
		return;
	} else if ((plo_syspage.entry < plo_syspage.kernel) || (plo_syspage.entry > plo_syspage.kernel + plo_syspage.ksize)) {
		plostd_printf(ATTR_ERROR, "\nPhoenix-RTOS start address is not valid.\n");
		return;
	}
	plostd_printf(ATTR_LOADER, "\nStarting Phoenix-RTOS...\n");

	f = (void*) plo_syspage.entry;
	low_cli();

	asm("ldr r8, =0x004F4C50");
	asm("ldr r10, =plo_syspage");
	asm("ldr r9,  [r10, #12]");
	asm("ldr r11, [r10, #16]");
	asm("add r10, r10, #20");

	f();
	low_sti();
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

static u32 chs2lba(CHS_t chs, int headsPerCylinder, int sectorsPerTrack) {
	u32 lba = 0;
	lba = (((chs.secCyl & 0xE) << 3) + ((chs.secCyl & 0xFF00) >> 8)) * headsPerCylinder;
	lba += chs.head;
	lba *= sectorsPerTrack;
	lba += (chs.secCyl & 0x1F) -1;
	return lba;
}

static int validMBR(MBR_t *mbr) {
	int p;
	if(mbr->signature != 0xAA55)
		return 0;

	for(p=0;p<4;++p) {
		if(mbr->pe[p].status != 0 && mbr->pe[p].status != 0x80)
			return 0;
		if(mbr->pe[p].status == 0x80 && (chs2lba(mbr->pe[p].firstSector, 1, 1) >= chs2lba(mbr->pe[p].lastSector, 1, 1)))
			return 0;
	}
	return 1;
}

static int validPartition(PartitionEntry_t *pe) {
	if(pe->partitionType == 0)
		return 0;

	//TODO - check if partition entry is contained by device
	/* it might be useful to check partition type - if it's accessible through CHS or LBA */
	return 1;
}

static int cmd_check_kernel(KernelDesc_t *kd, u64 *cseq, u64 *co, int pdn, s32 h, u64 blk)
{

	int idx;
	const char *phname = KERNEL_ID;

	u64 size = kd->size;
	u64 offs = kd->kern_offs;
	u64 seqnum = kd->kern_seqnum;

	/* check name */
	for(idx = 0; (idx < 8) && (phname[idx] == kd->name[idx]); ++idx);

	if(idx < 7) {
		kd->name[7] = 0;
		return -1;
	}

	plostd_printf(ATTR_ERROR, "Checking kernel %s seqnum %d.\n", kd->name, (int) kd->kern_seqnum);

	if(seqnum < *cseq)
		return -1;

	if(verifySignature(kd->sign_type, kd->signature, blk+offs, size, pdn, h) < 0) {
		plostd_printf(ATTR_ERROR, "Bad %s signature.\n", kd->sign_type);
		return -1;
	}
	if(*cseq <= seqnum) {
		*cseq = seqnum;
		*co = blk+offs;
		return 0;
	}

	return -1;
}

/**
 *
 * @param pdn - device ID (FLASH, MMC, AUTO)
 * @param h - handle to open device specified as pdn
 * @param pn - returned device (partition) name used to boot from
 * @param img - image number if there are multiole images on the device
 * @return offset of the kernel location
 */
static u64 cmd_find_kernel(const unsigned devnum, const s32 handle, const char *devname, int *img)
{
	MBR_t bootrec;
	u64 ver = 0, offset = 0;
	int ret = 3 * 512;
	KernelDesc_t kd;
	int i, partnum, lp = 0;

	*img = -1;
	if (devnum == PDN_FLASH) {
		static const unsigned flash_offsets[] = FLASH_KERNEL_OFFSETS;

		i = 0;
		do {
			offset = flash_offsets[i];
			if (phfs_read(devnum, handle, &offset, (u8*)&kd, 512) < 0) {
				plostd_printf(ATTR_ERROR, "Can't read device block!\n");
				return ret;
			}
			cmd_check_kernel(&kd, &ver, &offset, devnum, handle, offset / 512);
			i++;
		}
		while (flash_offsets[i] > flash_offsets[i - 1]);

		if (offset > 0)
			*img = i;

		return offset * 512;
	}
	else if (devnum == PDN_MMCBLK) {
		/* 1 - 4 for primary partition, 5+ for logical partition of extended partition*/
		partnum = plostd_atoi(devname + sizeof("mmcblk0p") - 1);

		if (phfs_read(devnum, handle, &offset, (u8*)&bootrec, 512) < 0) {
			plostd_printf(ATTR_ERROR, "Can't read mbr block!\n");
			return ret;
		}
		if(!validMBR(&bootrec))
			return ret;

		/* booting from primary partition */
		if (partnum < 5) {
			offset = (bootrec.pe[partnum - 1].firstLBA + 3) * 512;
			if (phfs_read(devnum, handle, &offset, (u8 *)&kd, 512) < 0) {
				plostd_printf(ATTR_ERROR, "Can't read device block!\n");
				return ret;
			}
			if (cmd_check_kernel(&kd, &ver, &offset, devnum, handle, offset / 512) == 0)
				*img = partnum;
		}
		else {
			/* booting from logical partition, first find extended partition */
			for (i = 0; i < 4; i++) {
				if (validPartition(&bootrec.pe[i]) && (bootrec.pe[i].partitionType == 0x05 ||
					bootrec.pe[i].partitionType == 0x0f || bootrec.pe[i].partitionType == 0x1f ||bootrec.pe[i].partitionType == 0x85))
				{
					lp = bootrec.pe[i].firstLBA;
					break;
				}
			}
			if (!lp) {
				plostd_printf(ATTR_ERROR, "Can't find any extended partition!\n");
				return ret;
			}

			offset = lp * 512;
			i = 4;

			do {
				if (phfs_read(devnum, handle, &offset, (u8 *)&bootrec, 512) < 0) {
					plostd_printf(ATTR_ERROR, "Can't read device block!\n");
					return ret;
				}
				if (!validMBR(&bootrec) || !validPartition(&bootrec.pe[0]))
					break;
				i++;

				if(!validPartition(&bootrec.pe[1]))
					break;
				offset = (lp + bootrec.pe[1].firstLBA) * 512;
			}
			while (i < partnum);

			if (i == partnum) {
				offset = (bootrec.pe[0].firstLBA + 3 + lp) * 512;
				if (phfs_read(devnum, handle, &offset, (u8 *)&kd, 512) < 0) {
					plostd_printf(ATTR_ERROR, "Can't read device block!\n");
					return ret;
				}

				if (cmd_check_kernel(&kd, &ver, &offset, devnum, handle, offset / 512) == 0)
					*img = partnum;
			}
		}

		if(offset) {
			plostd_printf(ATTR_ERROR, "Loading kernel version %d.\n", ver);
			return offset * 512;
		}
		return ret;
	}
}


void cmd_loadkernel(unsigned int pdn, const char *devname)
{
	char *path;
	s32 h;
	u32 loffs;
	u64 p, base;
	Elf32_Ehdr hdr;
	Elf32_Phdr phdr;
	Elf32_Word i, k;
	Elf32_Half seg, offs;
	Elf32_Word size, l;
	u8 buff[384];
	char image[10];
	int err;
	int argv_size;
	int img;
	u32 minaddr = 0xffffffff, maxaddr = 0;

	plostd_printf(ATTR_LOADER, "\n");

	if ((h = phfs_open(pdn, NULL, 0)) < 0) {
		plostd_printf(ATTR_ERROR, "Can't open device %s\n", devname);
		return;
	}

	base = cmd_find_kernel(pdn, h, devname, &img);
	if (img < 0) {
		plostd_printf(ATTR_ERROR, "Can't find kernel image!\n");
		return;
	}

	/* Read ELF header */
	p = base;
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
		p = base + hdr.e_phoff + k * sizeof(Elf32_Phdr);
		if (phfs_read(pdn, h, &p, (u8 *)&phdr, (u32)sizeof(Elf32_Phdr)) < 0) {
			plostd_printf(ATTR_ERROR, "Can't read Elf32_Phdr, k=%d!\n", k);
			return;
		}

		if ((phdr.p_type == PT_LOAD) && (phdr.p_paddr != 0)) {

			/* Calculate kernel memory parameters */
			if (minaddr > phdr.p_paddr)
				minaddr = phdr.p_paddr;
			if (maxaddr < phdr.p_paddr + phdr.p_memsz)
				maxaddr = phdr.p_paddr + phdr.p_memsz;

			loffs = phdr.p_paddr;
			plostd_printf(ATTR_LOADER, "Reading segment %p and writing it to %p:  ", phdr.p_paddr, loffs);

			for (i = 0; i < phdr.p_filesz / sizeof(buff); i++) {

				p = base + phdr.p_offset + i * sizeof(buff);
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
				p = base + phdr.p_offset + i * sizeof(buff);
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

	plo_syspage.entry = hdr.e_entry; /* set kernel entry point */
	plo_syspage.kernel = minaddr;
	plo_syspage.ksize = maxaddr - minaddr;
	plo_syspage.argc = 0;
	plo_syspage.argsize = 0;
	plostd_itoa(img, image, 0);
	add_kernel_arg("PARTITION", devname);
	add_kernel_arg("BOOT_IMAGE", image);
	add_default_kernel_args(DEFAULT_KERNEL_ARGS);
}


void cmd_load(char *s)
{
	const char *devname;
	u16 devnum;
	char word[LINESZ + 1];
	unsigned p = 0;
	unsigned dn;

	plostd_printf(ATTR_LOADER, "\n");
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
		if (!plostd_strcmp(word, devices[dn].name)) {
			devnum = devices[dn].pdn;
			break;
		}
	}

	if (!devices[dn].name) {
		plostd_printf(ATTR_ERROR, "\n'%s' - unknown boot device!\n", word);
		return;
	}
	if (devnum == PDN_AUTO) {
		devname = bootlog_dev();
		if (plostd_strstr(devname, "mtd") == devname)
			devnum = PDN_FLASH;
		else if (plostd_strstr(devname, "mmcblk") == devname)
			devnum = PDN_MMCBLK;
		else {
			plostd_printf(ATTR_ERROR, "Unkonown device!\n");
			return;
		}
	}
	else
		devname = word;

	plostd_printf(ATTR_LOADER, "Selected boot device: %s\n", devname);

	cmd_loadkernel(devnum, devname);
}


void cmd_reset(char *s)
{
	plostd_printf(ATTR_LOADER, "\n");
	low_reset();
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
