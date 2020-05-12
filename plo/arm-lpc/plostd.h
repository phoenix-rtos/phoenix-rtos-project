/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Standard functions
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
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

#ifndef _PLOSTD_H_
#define _PLOSTD_H_


#define NULL ((void *) 0)


extern int plostd_strlen(char *s);

extern int plostd_strcmp(char *s1, char *s2);

extern char *plostd_itoa(unsigned int i, char *buff, int x);

extern char *plostd_itoah(unsigned int i, char *buff, int lz);

extern unsigned int plostd_ahtoi(char *s);

extern void plostd_puts(char attr, char *s);

extern void plostd_printf(char attr, char *, ...);


#endif
