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

#ifndef _RHCOMMON_H_
#define _RHCOMMON_H_


#include <stdio.h>
#include <sys/threads.h>


#define BENCH_NAME(name) \
	do { \
		printf("Rhealstone benchmark suite\n%s\n", name); \
	} while (0)

uint64_t printResult(uint64_t start, uint64_t end, int loops, uint64_t loopOverhead, uint64_t singleOverhead);


static inline uint64_t getCntr(void)
{
	uint32_t asr22, asr23;
	uint64_t cntr;
	__asm__ volatile(
			"rd %%asr22, %0\n\t"
			"rd %%asr23, %1\n\t"
			: "=r"(asr22), "=r"(asr23) :);

	cntr = ((uint64_t)(asr22 & 0xffffffu) << 32) | asr23;

	return cntr;
}


uint64_t threadJoinOverhead(void);


uint64_t mutexLockOverhead(handle_t mutex);


#endif
