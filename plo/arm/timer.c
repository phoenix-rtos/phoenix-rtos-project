/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * System timer driver
 *
 * Copyright 2014 Phoenix Systems
 * Author: Katarzyna Baranowska
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

#include "types.h"
#include "timer.h"
#include "plostd.h"
#include "low.h"
#include "MVF50GS10MK50.h"

struct time_s {
	u32 sec;
	u32 msec;
};
typedef struct time_s time_t;

void timer_gettime(time_t *t)
{
	t->sec = 0xFFFFFFFF - PIT->CHANNEL[2].CVAL;
	t->msec = 1000 - PIT->CHANNEL[1].CVAL;
}

int timer_wait(u16 ms, int flags, volatile u16 *p, u16 v, u16 *ms_left)
{
	time_t timeout, now;

	timer_gettime(&timeout);
	timeout.msec += ms;
	timeout.sec += timeout.msec / 1000;
	timeout.msec = timeout.msec % 1000;

	for (;;) {
		timer_gettime(&now);
		if (now.sec > timeout.sec)
			break;
		if ((now.sec == timeout.sec) && (now.msec > timeout.msec))
			break;

		if (((flags & TIMER_KEYB) && low_keypressed()) ||
				((flags & TIMER_VALCHG) && (*p != v)))
			return 1;
	}
	if (ms_left != NULL) {
		*ms_left = timeout.sec - now.sec * 1000 + timeout.msec - now.msec;
	}
	return 0;
}

#define PIT_MCR_MDIS	0x2	/* PIT module disabled */
#define PIT_MCR_FRZ		0x1	/* Timers frozen when in debug mode */
#define PIT_TCTRL_CHN	0x4 /* chain with previous timer (timer 0 can't be chained) */
#define PIT_TCTRL_TIE	0x2	/* interrupt enable */
#define PIT_TCTRL_TEN	0x1	/* enable */
#define PIT_TFLG_TIF	0x1	/* interrupt pending flag (write 1 to clear) */

void timer_init(void)
{
	PIT->MCR = (PIT->MCR & ~PIT_MCR_MDIS) | PIT_MCR_FRZ;
	PIT->CHANNEL[2].TCTRL  = 0;
	PIT->CHANNEL[2].LDVAL  = 0xFFFFFFFF;
	PIT->CHANNEL[2].TFLG   = PIT_TFLG_TIF;
	PIT->CHANNEL[2].TCTRL  = PIT_TCTRL_CHN;
	PIT->CHANNEL[2].TCTRL |= PIT_TCTRL_TEN;

	PIT->CHANNEL[1].TCTRL  = 0;
	PIT->CHANNEL[1].LDVAL  = 1000;
	PIT->CHANNEL[1].TFLG   = PIT_TFLG_TIF;
	PIT->CHANNEL[1].TCTRL  = PIT_TCTRL_CHN;
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN;

	PIT->CHANNEL[0].TCTRL  = 0;
	PIT->CHANNEL[0].LDVAL  = (BUS_CLK_KHZ * 1024) / 1000;
	PIT->CHANNEL[0].TFLG   = PIT_TFLG_TIF;
	PIT->CHANNEL[0].TCTRL  = PIT_TCTRL_TEN;
}

void timer_done(void) {
	PIT->CHANNEL[0].TCTRL  = 0;
	PIT->CHANNEL[0].LDVAL  = 0;
	PIT->CHANNEL[0].TFLG   = PIT_TFLG_TIF;

	PIT->CHANNEL[1].TCTRL  = 0;
	PIT->CHANNEL[1].LDVAL  = 0;
	PIT->CHANNEL[1].TFLG   = PIT_TFLG_TIF;

	PIT->CHANNEL[2].TCTRL  = 0;
	PIT->CHANNEL[2].LDVAL  = 0;
	PIT->CHANNEL[2].TFLG   = PIT_TFLG_TIF;

	PIT->MCR = 0;
}
