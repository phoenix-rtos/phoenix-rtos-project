/*
 * Phoenix-RTOS
 *
 * Rhealstone benchmark
 *
 * Task switch
 *
 * Copyright 2024 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/threads.h>
#include <sys/minmax.h>

#include <rhcommon.h>

#define MAX_LOOPS 10000

#define ONE_TICK     26000 /* Number dependent on CPU. For loop to this count must be longer than sleep period (1 ms) */
#define ONE_TICK_AVG 24990 /* For loop up to this count takes 1ms */

unsigned char stack[5][4096];
volatile atomic_int delay = 0;

uint64_t start1, start2, end1, end2;


/* Low priority */
void task1(void *arg)
{
	start1 = getCntr();
	for (volatile int cnt1 = 0; cnt1 < MAX_LOOPS; cnt1++) {
		for (delay = 0; delay < ONE_TICK; delay++) {
			__asm__ volatile("nop");
			/* Delay loop */
		}
	}
	end1 = getCntr();
	endthread();
}

/* High priority */
void task2(void *arg)
{
	start2 = getCntr();
	for (volatile int cnt2 = 0; cnt2 < MAX_LOOPS; cnt2++) {
		delay = ONE_TICK;
		usleep(1000);
	}
	end2 = getCntr();
	endthread();
}


int main(void)
{
	BENCH_NAME("Preemption");

	priority(1);

	uint64_t overhead = getCntr();

	for (volatile int cnt1 = 0; cnt1 < MAX_LOOPS; cnt1++) {
		for (volatile int i = 0; i < ONE_TICK_AVG; i++) {
			/* Delay loop */
		}
	}
	for (int cnt2 = 0; cnt2 < MAX_LOOPS; cnt2++) {
		__asm__ volatile("nop");
	}

	overhead = getCntr() - overhead;

	int tid1, tid2;
	int res = beginthreadex(task1, 3, stack[0], sizeof(stack[0]), NULL, &tid1);
	if (res < 0) {
		perror("beginthreadex");
		return -1;
	}

	res = beginthreadex(task2, 2, stack[1], sizeof(stack[1]), NULL, &tid2);
	if (res < 0) {
		perror("beginthreadex");
		return -1;
	}

	priority(4);

	usleep(0);

	threadJoin(tid1, 0);
	threadJoin(tid2, 0);

	printResult(min(start1, start2), max(end1, end2), 2 * MAX_LOOPS, overhead, 0);

	return 0;
}
