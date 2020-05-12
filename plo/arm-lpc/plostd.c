/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Standard functions
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

#include "errors.h"
#include "types.h"
#include "low.h"
#include "plostd.h"

const char digits[] = "0123456789";
const char digitsh[] = "0123456789abcdef";


int plostd_strlen(char *s)
{
	char *p;
	unsigned int k = 0;

	for (p = s; *p; p++)
		k++;
	return k;
}


int plostd_strcmp(char *s1, char *s2)
{
	unsigned int k = 0;
	for (k = 0; s1[k] && s2[k]; k++) {
		if (s1[k] != s2[k])
			break;
	}
	if (s1[k] || s2[k])
		return ERR_STD_NOTEQ;
	return 0;
}


char *plostd_itoa(unsigned int i, char *buff, int x)
{
	int l = 0;
	int div, offs;
	int nz = 0;

	switch (sizeof(i)) {
	case 1:
		div = 100;
		break;
	case 2:
		div = 10000;
		break;
	case 4:
		div = 1000000000;
		break;
	default:
		return NULL;
	}

	while (div) {
		if ((offs = i / div) != 0)
			nz = 1;
		if (nz) {
			buff[l] = digitsh[offs];
			l++;
		}
		i -= offs * div;
		div /= 10;
	}
	if (!l)
		buff[l++] = digitsh[offs];

	buff[l] = 0;
	return buff;
}


char *plostd_itoah(unsigned int i, char *buff, int lz)
{
	int l, offs, k, shn;
	int nz = 0;

	switch (sizeof(i)) {
	case 1:
		shn = 2;
		break;
	case 2:
		shn = 4;
		break;
	case 4:
		shn = 8;
		break;
	default:
		return NULL;
	}

	for (k = 0, l = 0; k < shn; k++) {
		if ((offs = ((i >> ((shn - 1) * 4 - 4 * k)) & 0xf)) != 0)
			nz = 1;
		if (lz || nz) {
			buff[l] = digitsh[offs];
			l++;
		}
	}

	if (!lz && !l){
		buff[l++] = digitsh[offs];
	}

	buff[l] = 0;
	return buff;
}


unsigned int plostd_ahtoi(char *s)
{
	char *p;
	int k, i, found;
	unsigned int v, pow;

	v = 0;
	pow = 0;
	for (k = plostd_strlen(s) - 1; k >= 0; k--, pow++) {
		p = (char *)(s + k);
		if ((*p == ' ') || (*p == '\t'))
			continue;

		found = 0;
		for (i = 0; i < 16; i++)
			if (digitsh[i] == *(char *)p) {
				found = 1;
				break;
			}
		if (!found)
			return 0;

		if (pow > (sizeof(int) * 2 - 1))
			return v;
		v += (i << (pow * 4));
	}
	return v;
}


void plostd_puts(char attr, char *s)
{
	char *p;

	for (p = s; *p; p++)
		low_putc(attr, *p);
	return;
}


void plostd_printf(char attr, char *fmt, ...)
{
	va_list ap;
	char *p;
	char buff[16];

	ap = (u8 *)&fmt + sizeof(fmt);

	for (p = fmt; *p; p++) {
		if (*p != '%') {
			low_putc(attr, *p);
			continue;
		}

		switch (*++p) {
		case 'd':
			plostd_puts(attr, plostd_itoa(va_arg(ap, int), buff, 0));
			break;
		case 'x':
			plostd_puts(attr, plostd_itoah(va_arg(ap, int), buff, 0));
			break;
		case 'p':
			plostd_puts(attr, plostd_itoah(va_arg(ap, int), buff, 1));
			break;
		case 's':
			plostd_puts(attr, va_arg(ap, char *));
			break;
		case 'c':
			low_putc(attr, va_arg(ap, int));
			break;
		case '%':
			low_putc(attr, '%');
			break;
		}
	}
	va_end(ap);

	return;
}
