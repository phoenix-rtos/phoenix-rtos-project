/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Configuration
 *
 * Copyright 2012, 2020 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_


#define VERSION      "1.3"


/* User interface */
#define WELCOME      "-\\- Phoenix-RTOS loader, version: " VERSION
#define PROMPT       "(plo)% "

#define LINESZ       80
#define HISTSZ       8


/* Console character attributes */
#define ATTR_DEBUG   2
#define ATTR_USER    7
#define ATTR_INIT    13
#define ATTR_LOADER  15
#define ATTR_ERROR   4


/* Kernel file */
#define KERNEL_PATH  "phoenix"

/* Kernel args */
#define KERNEL_ARGS  "Xpinit"

/* Kernel adresses and sizes */
#define KERNEL_BASE  0xc0000000

#define GDT_SEG      0x0100
#define GDT_SIZE     0x0800

#define IDT_SEG      0x0180
#define IDT_SIZE     0x0800

#define SYSPAGE_SEG  0x0200
#define SYSPAGE_SIZE 0x1000

#define PDIR_SEG     0x0300
#define PTABLE_SEG   0x0400

#define INIT_ESP     0x6000
#define STACK_SIZE   0x1000


/* Boot command size */
#define CMD_SIZE     64


/* Disk cache */
#define SECTOR_SIZE  512
#define CACHE_SIZE   32
#define CACHE_OFFS   0x9000


#endif
