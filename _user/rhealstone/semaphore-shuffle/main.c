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
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/threads.h>

#include <rhcommon.h>


#define BENCHMARKS 500000


bool semExe = false;
atomic_int count = 0, ready = 0;
handle_t mutex;
uint64_t overhead = 0;

unsigned char stack[2][4096];


void task2(void *arg)
{
	uint64_t start, end;
	start = getCntr();

	for (count = 0; count < BENCHMARKS; count++) {
		if (semExe) {
			mutexLock(mutex);
		}

		usleep(0);

		if (semExe) {
			mutexUnlock(mutex);
		}

		usleep(0);
	}
	end = getCntr();

	if (!semExe) {
		overhead = end - start;
	}
	else {
		printResult(start, end, BENCHMARKS, overhead, 0);
	}
	endthread();
}


void task1(void *arg)
{
	int tid;
	if (beginthreadex(task2, 2, stack[1], sizeof(stack[1]), NULL, &tid)) {
		perror("beginthreadex");
		endthread();
	}

	usleep(0);
	for (; count < BENCHMARKS;) {
		if (semExe) {
			mutexLock(mutex);
		}

		usleep(0);

		if (semExe) {
			mutexUnlock(mutex);
		}

		usleep(0);
	}

	threadJoin(tid, 0);

	endthread();
}


int main(void)
{
	BENCH_NAME("Semaphore shuffle");

	priority(1);

	mutexCreate(&mutex);

	int tid1;
	if (beginthreadex(task1, 2, stack[0], sizeof(stack[0]), NULL, &tid1) < 0) {
		perror("beginthreadex");
		return -1;
	}

	priority(3);

	usleep(0);

	threadJoin(tid1, 0);

	priority(1);

	semExe = true;

	if (beginthreadex(task1, 2, stack[0], sizeof(stack[0]), NULL, &tid1) < 0) {
		perror("beginthreadex");
		return -1;
	}

	priority(3);

	usleep(0);

	threadJoin(tid1, 0);

	return 0;
}
