/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Low-level routines
 *
 * Copyright 2012, 2020 Phoenix Systems
 * Copyright 2001, 2005, 2012 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _LOW_H_
#define _LOW_H_

#include "types.h"


/* Low-level loader variables */
extern u16 _plo_timeout;

extern u8 _plo_baid;

extern void _plo_command(void);

extern int _plo_save(void);


typedef struct {
	u32 addr_lo;
	u32 addr_hi;
	u32 len_lo;
	u32 len_hi;
	u32 attr;
} low_mmitem_t;


typedef struct syspage_program_t {
	u32 entry;
	u32 *hdrssz;
	struct {
		u32 addr;
		u32 size;
		u32 flags;
		u32 vaddr;
		u32 filesz;
		u32 padding;
	} hdrs[];
} syspage_program_t;


#define MM_MAXSZ   64

typedef struct _syspage_t {
	u8 gdtr[8];
	u8 idtr[8];
	u32 pdir;
	u32 ptable;

	u32 stack;
	u32 stacksize;

	u32 kernel;
	u32 kernelsize;
	u32 console;
	char arg[256];

	u16 mmsize;
	low_mmitem_t mm[MM_MAXSZ];

	u16 progssz;
	syspage_program_t progs[];
} syspage_t;


#define SYSPAGE_OFFS_GDTR        0
#define SYSPAGE_OFFS_IDTR        8
#define SYSPAGE_OFFS_PDIR        (8 + 8)
#define SYSPAGE_OFFS_PTABLE      (8 + 8 + 4)
#define SYSPAGE_OFFS_STACK       (8 + 8 + 4 + 4)
#define SYSPAGE_OFFS_STACKSIZE   (8 + 8 + 4 + 4 + 4)
#define SYSPAGE_OFFS_KERNEL      (8 + 8 + 4 + 4 + 4 + 4)
#define SYSPAGE_OFFS_KERNELSIZE  (8 + 8 + 4 + 4 + 4 + 4 + 4)
#define SYSPAGE_OFFS_CONSOLE     (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4)
#define SYSPAGE_OFFS_ARG         (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4)
#define SYSPAGE_OFFS_MMSIZE      (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 256)
#define SYSPAGE_OFFS_MM          (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 256 + 2)
#define SYSPAGE_OFFS_PROGSSZ     (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 256 + 2 + MM_MAXSZ * 20)
#define SYSPAGE_OFFS_PROGS       (8 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 256 + 2 + MM_MAXSZ * 20 + 2)

#define IRQ_HANDLED   0
#define IRQ_DEFAULT   1


#define min(a, b)   ((a > b) ? b : a)


extern void low_cli(void);

extern void low_sti();

extern u8 low_inb(u16 addr);

extern void low_outb(u16 addr, u8 b);

extern u32 low_ind(u16 addr);

extern void low_outd(u16 addr, u32 d);


extern void low_setfar(u16 segm, u16 offs, u16 v);

extern u16 low_getfar(u16 segm, u16 offs);

extern void low_setfarbabs(u32 addr, u8 v);

extern u8 low_getfarbabs(u32 addr);

extern void low_setfarabs(u32 addr, u32 v);

extern u32 low_getfarabs(u32 addr);

extern void low_copyto(u16 segm, u16 offs, void *src, unsigned int l);

extern void low_copyfrom(u16 segm, u16 offs, void *dst, unsigned int l);

extern void low_memcpy(char *dst, char *src, unsigned int l);

extern void low_copytoabs(u32 addr, void *src, unsigned int l);

extern void low_copyfromabs(u32 addr, void *dst, unsigned int l);

extern u16 low_getcs(void);


extern void low_setmode(u8 m);

extern void low_putc(char attr, char c);

extern void low_getc(char *c, char *sc);

extern int low_keypressed(void);

extern int low_int13read(u8 drive, u16 c, u8 h, u8 s, u8 n, u8 *buff);

extern int low_int13param(u8 drive, u16 *mc, u8 *mh, u8 *ms);


/* Function prepares memory map for kernel */
extern int low_mmcreate(void);

/* Function returns selected memory map item */
extern int low_mmget(unsigned int n, low_mmitem_t *mmitem);

/* Function starts kernel loaded into memory */
extern int low_launch(void);


extern void low_maskirq(u16 n, u8 v);

extern int low_irqinst(u16 irq, int (*isr)(u16, void *), void *data);

extern int low_irquninst(u16 irq);


/* Function enables upper memory access in the real mode (a.k.a big unreal mode) */
extern void low_unreal(void);

/* Function enables A20 line */
extern u8 low_a20(void);

extern void low_init(void);

extern void low_done(void);


#endif
