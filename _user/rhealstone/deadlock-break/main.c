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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/threads.h>

#include <board_config.h>
#include <rhcommon.h>


#define BENCHMARKS 10000

bool deadBrk;
volatile bool done = false;
handle_t mutex;

unsigned char stack[3][4096];
int maxLoops = BENCHMARKS;
int tid1, tid2, tid3;

atomic_bool t3_started = false;

uint64_t totalNoDeadBrk = 0;
uint64_t totalDeadBrk = 0;

/* High priority task */
void task3(void *arg)
{
	t3_started = true;
	uint64_t time = getCntr();
	if (deadBrk) {
		mutexLock(mutex);
	}

	if (deadBrk) {
		mutexUnlock(mutex);
		time = getCntr() - time;
		totalDeadBrk += time;
	}
	else {
		time = getCntr() - time;
		totalNoDeadBrk += time;
	}
	done = true;
	endthread();
}


/* Medium priority task */
void task2(void *arg)
{
	if (beginthreadex(task3, 1, stack[2], sizeof(stack[2]), NULL, &tid3)) {
		perror("beginthreadex");
		endthread();
	}

	while (!done) {
		__asm__ volatile("nop");
	}

	threadJoin(tid3, 0);

	endthread();
}

/* Low priority task */
void task1(void *arg)
{
	if (deadBrk) {
		mutexLock(mutex);
	}

	if (beginthreadex(task2, 2, stack[1], sizeof(stack[1]), NULL, &tid2)) {
		perror("beginthreadex");
		endthread();
	}

	while (!t3_started) {
		usleep(0);
	}

	if (deadBrk) {
		mutexUnlock(mutex);
	}

	threadJoin(tid2, 0);
	t3_started = false;
	endthread();
}


int doTest(void)
{
	if (beginthreadex(task1, 3, stack[0], sizeof(stack[0]), NULL, &tid1)) {
		perror("beginthreadex");
		return -1;
	}

	priority(4);

	usleep(0);

	threadJoin(tid1, 0);

	priority(0);

	return 0;
}


int main(int argc, char *argv[])
{
	BENCH_NAME("Deadlock Break");

	if (argc > 1) {
		maxLoops = atoi(argv[1]);
	}

	priority(0);

	mutexCreate(&mutex);

	uint64_t mutexOverhead = mutexLockOverhead(mutex);
	uint64_t joinOverhead = threadJoinOverhead();

	deadBrk = false;


	for (int i = 0; i < maxLoops; i++) {
		if (doTest() < 0) {
			return -1;
		}
	}

	printf("No deadlock: \n");
	printResult(0, totalNoDeadBrk, 1, joinOverhead, 0);

	deadBrk = true;

	for (int i = 0; i < maxLoops; i++) {
		if (doTest() < 0) {
			return -1;
		}
	}

	printf("Deadlocks: full time\n");
	printResult(0, totalDeadBrk, 1, joinOverhead, 0);

	printf("Deadlocks: per resolution\n");
	printResult(0, totalDeadBrk, maxLoops, totalNoDeadBrk, mutexOverhead);

	return 0;
}
