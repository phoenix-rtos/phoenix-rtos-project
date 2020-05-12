/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Low-level routines
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

#ifndef _LOW_H_
#define _LOW_H_

#include "types.h"

#define min(a, b)   ((a > b) ? b : a)

typedef struct __attribute__((packed)) _plo_syspage_t {
	u32 entry;
	u32 kernel;
	u32 ksize;
	u32 argc;
	u32 argsize;
	char args[256];
} plo_syspage_t;

typedef struct{
	u32 magic;
	u32 timeout;
	char command[64];
	u8 pad[256-64-4-4];
}pdata_t;

extern time_st now;
extern plo_syspage_t plo_syspage;
extern u16 _plo_timeout;
extern char _plo_command[];
extern pdata_t *pdata_ptr;


extern void low_init(void);
extern void low_cli(void);
extern void low_sti(void);
extern int low_keypressed();
extern void low_memcpy(void *dst_, const void *src_, unsigned int l);
extern void low_memset(void *dst_, char c, unsigned int l);
extern void low_putc(const char attr, const char c);
extern void low_getc(char *c, char *sc);
extern void low_irqinst(u8, u16, int (*)(u16, void*), void*);
extern void low_irquninst(u16);
extern void low_irqen(u16, u8);
extern void low_done(void);
extern void low_dbg(u8);

extern void low_getregs(int*, int*);
extern void irq_enable(void);
extern void irq_disable(void);

extern void low_ddrInit(void);
extern void low_clocksInit(void);

extern u32 low_persistGet();
extern void low_persistSet(const u32 val);
extern u16 low_crc16(u16 *data, unsigned count);
extern u32 low_crc32(u32 data[], unsigned count);
extern void low_reset(void);
#endif
