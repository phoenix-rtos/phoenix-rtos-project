/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
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


int plostd_strlen(const char *s)
{
	const char *p;
	unsigned int k = 0;

	for (p = s; *p; p++)
		k++;
	return k;
}


int plostd_strcmp(const char *s1, const char *s2)
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


char *plostd_strstr(const char *s1, const char *s2)
{
	unsigned i, j;

	for (i = 0; s1[i]; i++) {
		for (j = 0; s1[i + j] && s2[j]; j++)
			if (s1[i + j] != s2[j])
				break;
		if (s2[j] == '\0')
			return (char *)s1 + i;
	}
	return NULL;
}


char *plostd_itoa(unsigned int i, char *buff, int x)
{
	int l = 0;
	int div, offs;
	int nz = 0;
	char c;

	if(buff == 0)
		return buff;

	if (i == 0) {
		buff[0] = '0';
		buff[1] = 0;
		return buff;
	}
	while (i != 0) {
		buff[l++] = i % 10 + '0';
		i = i / 10;
	}

	for (i = 0; i < l / 2; i++) {
		c = buff[i];
		buff[i] = buff[l-i-1];
		buff[l-i-1] = c;
	}
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


unsigned int plostd_ahtoi(const char *s)
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

int plostd_atoi(const char *s)
{
	int i = 0, sign = 1;
	const char *p = s;

	while (*p) {
		if (*p >= '0' && *p <= '9')
			break;
		if (*p == '-') {
			sign = -1;
			p++;
			break;
		}
		if (*p != ' ')
			return 0;
		p++;
	}

	while (*p) {
		if (*p >= '0' && *p <= '9')
			i = i * 10 + *p - '0';
		else
			break;
		p++;
	}

	return sign * i;
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
