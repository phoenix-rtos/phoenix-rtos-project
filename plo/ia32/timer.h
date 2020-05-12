/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * System timer driver
 *
 * Copyright 2012, 2020 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _TIMER_H_
#define _TIMER_H_


#define TIMER_EXPIRE   0
#define TIMER_KEYB     2
#define TIMER_VALCHG   4


extern void timer_cycles(u64 *c);

extern void timer_cyclesdiff(u64 *c1, u64 *c2, u64 *res);

extern int timer_wait(u16 ms, int flags, u16 *p, u16 v);

extern void timer_init(void);

extern void timer_done(void);


#endif
