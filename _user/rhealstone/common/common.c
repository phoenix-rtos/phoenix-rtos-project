/*
 * Phoenix-RTOS
 *
 * Rhealstone benchmark
 *
 * Common functions
 *
 * Copyright 2024 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "rhcommon.h"

#include <stdatomic.h>
#include <stdio.h>
#include <sys/threads.h>
#include <unistd.h>


uint64_t printResult(uint64_t start, uint64_t end, int loops, uint64_t loopOverhead, uint64_t singleOverhead)
{
	uint64_t elapsed = end - start - loopOverhead;
	float time = ((float)elapsed / (float)loops) - (float)singleOverhead;
	printf("Result: %.1f cycles\n", time);

	return elapsed;
}


static void emptyThread(void *arg)
{
	endthread();
}


uint64_t threadJoinOverhead(void)
{
	uint64_t start, end;
	int tid;
	char stack[512];
	if (beginthreadex(emptyThread, 0, stack, sizeof(stack), NULL, &tid) < 0) {
		perror("beginthreadex");
		return 0;
	}

	usleep(100000);

	start = getCntr();
	threadJoin(tid, 0);
	end = getCntr();

	return end - start;
}


uint64_t mutexLockOverhead(handle_t mutex)
{
	const int loops = 100;
	uint64_t start, end, total = 0;

	for (int i = 0; i < loops; i++) {
		start = getCntr();
		mutexLock(mutex);
		end = getCntr();
		mutexUnlock(mutex);
		total += end - start;
	}

	return total / loops;
}
