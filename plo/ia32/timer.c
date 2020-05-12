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

#include "low.h"
#include "plostd.h"
#include "timer.h"


typedef struct {
	u16 jiffies;
} timer_t;


timer_t timer;


void timer_cycles(u64 *c)
{
#asm
	push bp
	mov bp, sp
	push di
	push edx

	mov di, 4[bp]
	; rdtsc
	db 0x0f, 0x31
	mov dword ptr [di], eax
	mov dword ptr 4[di], edx

	pop edx
	pop di
	pop bp
#endasm
}


static void timer_cyclesadd(u64 *c1, u64 *c2, u64 *res)
{
#asm
	push bp
	mov bp, sp
	push di
	push si

	mov si, 4[bp]
	mov eax, dword ptr [si]
	mov si, 6[bp]
	add eax, dword ptr [si]
	mov di, 8[bp]
	mov dword ptr [di], eax
	mov si, 4[bp]
	mov eax, dword ptr 4[si]
	mov si, 6[bp]
	adc eax, dword ptr 4[si]
	mov dword ptr 4[di], eax

	pop si
	pop di
	pop bp
#endasm
}


static void timer_cyclessub(u64 *c1, u64 *c2, u64 *res)
{
#asm
	push bp
	mov bp, sp
	push di
	push si

	mov si, 4[bp]
	mov eax, dword ptr [si]
	mov si, 6[bp]
	sub eax, dword ptr [si]
	mov di, 8[bp]
	mov dword ptr [di], eax
	mov si, 4[bp]
	mov eax, dword ptr 4[si]
	mov si, 6[bp]
	sbb eax, dword ptr 4[si]
	mov dword ptr 4[di], eax

	pop si
	pop di
	pop bp
#endasm
}


void timer_cyclesdiff(u64 *c1, u64 *c2, u64 *res)
{
	if ((c1->hi < c2->hi) || ((c1->hi == c2->hi) && (c1->lo < c2->lo))) {
		res->hi = 0xffffffff;
		res->lo = 0xffffffff;
		timer_cyclessub(res, c2, res);
		timer_cyclesadd(res, c1, res);
	}
	else {
		timer_cyclessub(c1, c2, res);
	}
}


static int timer_isr(u16 irq, void *data)
{
	timer_t *timer = data;

	timer->jiffies += 62;
	return IRQ_DEFAULT;
}


static int timer_valchg(u16 *p, u16 v)
{
#asm
	push bp
	mov bp, sp
	push di

	mov di, 4[bp]
	mov ax, 6[bp]
	sub ax, word ptr [di]

	pop di
	pop bp
#endasm
}


int timer_wait(u16 ms, int flags, u16 *p, u16 v)
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
			((flags & TIMER_VALCHG) && timer_valchg(p, v)))
			return 1;
	}
	return 0;
}


void timer_init(void)
{
	timer.jiffies = 0;
	low_irqinst(0, timer_isr, (void *)&timer);
	return;
}


void timer_done(void)
{
	low_irquninst(0);
	low_irquninst(1);
}
