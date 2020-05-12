/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Standard functions
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _PLOSTD_H_
#define _PLOSTD_H_


#define NULL    0


extern int plostd_strlen(char *s);

extern int plostd_strcmp(char *s1, char *s2);

extern char *plostd_itoa(unsigned int i, char *buff);

extern char *plostd_itoah(u8 *ip, u8 is, char *buff, int lz);

extern unsigned int plostd_ahtoi(char *s);

extern void plostd_puts(char attr, char *s);

extern void plostd_printf(char attr, char *, ...);


#endif
