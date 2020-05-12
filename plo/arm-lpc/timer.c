/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * System timer driver
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

#include "low.h"
#include "plostd.h"
#include "timer.h"

#define T0_BASE	0xe0004000

typedef struct {
	vu32	ir;
	vu32	tcr;
	vu32	tc;
	vu32	pr;
	vu32	pc;
	vu32	mcr;
	vu32	mr[4];
	vu32	ccr;
	vu32	cr[4];
	vu32	emr;
	vu32	ctcr;
}* timer_regs_t;

typedef struct {
	timer_regs_t cntl;
	u16 jiffies;
} timer_t;

timer_t timer;


int timer_isr(u16 irq, void *data)
{
	timer_t *timer = data;

	timer->cntl->ir = 0x01;
	timer->jiffies += 10;
	return 0;
}


int timer_wait(u16 ms, int flags, volatile u16 *p, u16 v)
{
	u16 current, dt;

	current = timer.jiffies;

	for (;;) {
		if (current > timer.jiffies)
			dt = 0xffff - current + timer.jiffies;
		else
			dt = timer.jiffies - current;

		if (ms && (dt >= ms))
			break;

		if (((flags & TIMER_KEYB) && low_keypressed()) ||
				((flags & TIMER_VALCHG) && (*p != v)))
			return 1;
	}
	return 0;
}


void timer_init(void)
{
	timer.jiffies = 0;
	timer.cntl = (timer_regs_t) T0_BASE;

	timer.cntl->tcr  = 0x2;		/* stop timer */
	timer.cntl->ir   = 0xff;		/* reset interrupts */
	timer.cntl->ctcr = 0x00;
	timer.cntl->pr   = 0x3a98;
	timer.cntl->mr[0] = 10;
	timer.cntl->mcr   = 0x3;

	low_cli();
	low_irqinst(4, 0, timer_isr,(void*) &timer);
	low_irqen(0, 1);
	low_sti();

	/* start timer */
	timer.cntl->tcr  = 0x1;

	return;
}


void timer_done(void)
{
	timer.cntl->tcr  = 0x2;
	low_cli();
	low_irqen(0, 0);
	low_irquninst(0);
	low_sti();
}
