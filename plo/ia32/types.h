/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Common types
 *
 * Copyright 2020 Phoenix Systems
 * Copyright 2001, 2005, 2012 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _TYPES_H_
#define _TYPES_H_


typedef unsigned char u8;
typedef unsigned int  u16;
typedef unsigned long u32;
typedef long          s32;

typedef struct {
	u32 lo;
	u32 hi;
} u64;


/* stdarg definitions */
typedef u8 *va_list;

#define va_start(ap, parmN) ((void)((ap) = (va_list)((char *)(&parmN) + sizeof(parmN))))
#define va_arg(ap, type) (*(type *)(((*(char **)&(ap)) += sizeof(type)) - (sizeof(type))))
#define va_end(ap) ((void)0)


#endif
