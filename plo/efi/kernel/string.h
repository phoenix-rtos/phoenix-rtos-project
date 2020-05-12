/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * HAL basic routines
 *
 * Copyright 2012, 2017 Phoenix Systems
 * Copyright 2001, 2005-2006 Pawel Pisarczyka
 *
 * Author: Pawel Pisarczyk, Michał Mirosław
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _HAL_BASE_H_
#define _HAL_BASE_H_

#include "cpu.h"


static inline void hal_memcpy(void *to, const void *from, unsigned int n)
{
	unsigned int rn;

	rn = n % 4;
	n /= 4;

	__asm__ volatile
	(" \
		cld; \
		rep; movsl"
	: "+S" (from), "+D" (to), "+c" (n)
	:
	: "memory");

	if (!__builtin_constant_p(rn) || rn)
		__asm__ volatile
		(
			"rep; movsb"
		: "+S" (from), "+D" (to), "+c" (rn)
		:
		: "memory");
}


static inline void hal_memset(void *where, u8 v, unsigned int n)
{
	unsigned int rn;
	u32 vvvv;

	rn = n % 4;
	n /= 4;

	vvvv = v | (v << 8);
	vvvv |= vvvv << 16;

	__asm__ volatile
	(" \
		cld; \
		rep; stosl"
	: "+D" (where), "+c" (n)
	: "a" (vvvv)
	: "memory");

	if (!__builtin_constant_p(rn) || rn)
		__asm__ volatile
		(
			"rep; stosb"
		: "+D" (where), "+c" (rn)
		: "a" (vvvv)
		: "memory");
}


static inline void hal_memsetw(void *where, u16 v, unsigned int n)
{
	unsigned int rn;
	u32 vv;

	rn = n % 2;
	n /= 2;

	vv = v | (v << 16);

	__asm__ volatile
	(" \
		cld; \
		rep; stosl"
	: "+D" (where), "+c" (n)
	: "a" (vv)
	: "memory");

	if (!__builtin_constant_p(rn) || rn)
		__asm__ volatile
		(
			"rep; stosw"
		: "+D" (where), "+c" (rn)
		: "a" (vv)
		: "memory");
}


static inline unsigned int hal_strlen(const char *s)
{
	unsigned int k;

	for (k = 0; *s; s++, k++);
	return k;
}


static inline int hal_strcmp(const char *s1, const char *s2)
{
	const char *p;
	unsigned int k;

	for (p = s1, k = 0; *p; p++, k++) {

		if (*p < *(s2 + k))
			return -1;
		else if (*p > *(s2 + k))
			return 1;
	}

	if (*p != *(s2 + k))
		return -1;

	return 0;
}


/* FIXME it should return -K or +K or 0  */
static inline int hal_strncmp(const char *s1, const char *s2, unsigned int count)
{
	const char *p;
	unsigned int k = 0;

	for (p = s1; *p; p++) {
		if ((*p != *(s2 + k)) && (k < count))
			return -1;
		k++;
	}

	if (k < count)
		return -1;

	return 0;
}


static inline char *hal_strcpy(char *dest, const char *src)
{
	int i = 0;

	do {
		dest[i] = src[i];
	} while(src[i++] != '\0');

	return dest;
}


static inline char *hal_strncpy(char *dest, const char *src, size_t n)
{
	int i = 0;

	do {
		dest[i] = src[i];
		i++;
	} while (i < n && src[i - 1] != '\0');

	return dest;
}


#endif
