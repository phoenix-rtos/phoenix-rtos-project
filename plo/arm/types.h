/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Common types
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

#ifndef _TYPES_H_
#define _TYPES_H_


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef short s16;
typedef int s32;
typedef int s32;
typedef long long s64;

typedef volatile unsigned char  vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int   vu32;

typedef vu8  *reg8;
typedef vu16 *reg16;
typedef vu32 *reg32;


/* stdarg definitions */
typedef u8 *va_list; 

#define va_start(ap, parmN) ((void)((ap) = (va_list)((char *)(&parmN) + sizeof(parmN))))
#define va_arg(ap, type) (*(type *)(((*(char **)&(ap)) += sizeof(type)) - (sizeof(type))))
#define va_end(ap) ((void)0)

typedef volatile struct {
	int sec, min, hour, dom, dow, doy, month, year;
} *time_st;

typedef union{
	u32 all;
	struct{
		u32 mode	: 5;
		u32 t_bit	: 1;
		u32 f_bit	: 1;
		u32 i_bit	: 1;
		u32 dmz		: 19;
		u32 q_bit	: 1;
		u32 flags	: 4;
	};
}psr_t;


#endif
