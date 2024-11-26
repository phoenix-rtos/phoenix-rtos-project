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

#include <rhcommon.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/threads.h>
#include <sys/minmax.h>

#define MAX_LOOPS 4000000


unsigned char stack[5][4096];
uint64_t start1, start2, end1, end2;


void task1(void *arg)
{
	start1 = getCntr();
	for (unsigned int i = 0; i < MAX_LOOPS; i++) {
		usleep(0);
	}
	end1 = getCntr();
	endthread();
}


void task2(void *arg)
{
	start2 = getCntr();
	for (unsigned int i = 0; i < MAX_LOOPS; i++) {
		usleep(0);
	}
	end2 = getCntr();
	endthread();
}


int main(void)
{
	printf("Task Switch benchmark\n");

	priority(1);

	uint64_t overhead = getCntr();

	for (unsigned int i = 0; i < MAX_LOOPS; i++) {
		__asm__ volatile("nop");
	}
	for (unsigned int i = 0; i < MAX_LOOPS; i++) {
		__asm__ volatile("nop");
	}

	overhead = getCntr() - overhead;

	int tid1, tid2;
	int res = beginthreadex(task1, 2, stack[0], sizeof(stack[0]), NULL, &tid1);
	if (res < 0) {
		perror("beginthreadex");
		return -1;
	}

	res = beginthreadex(task2, 2, stack[1], sizeof(stack[1]), NULL, &tid2);
	if (res < 0) {
		perror("beginthreadex");
		return -1;
	}

	priority(3);

	usleep(0);

	threadJoin(tid1, 0);
	threadJoin(tid2, 0);

	printResult(min(start1, start2), max(end1, end2), 2 * MAX_LOOPS, overhead, 0);

	return 0;
}
