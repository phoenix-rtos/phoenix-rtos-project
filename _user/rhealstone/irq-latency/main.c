/*
 * Phoenix-RTOS
 *
 * Rhealstone benchmark
 *
 * Message latency
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/interrupt.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/threads.h>

#include <board_config.h>
#include <rhcommon.h>


#define BENCHMARKS 10000

volatile uint64_t benchEnd = 0;

#ifdef __CPU_GR740
#define IRQ_UNUSED 13
#endif

uint64_t results[BENCHMARKS] = { 0 };


int irqHandler(unsigned int n, void *arg)
{
	benchEnd = getCntr();

	return 0;
}


int main(void)
{
	BENCH_NAME("Interrupt latency");

	volatile uint32_t *irqCtrl = mmap(NULL, _PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_DEVICE | MAP_PHYSMEM | MAP_ANONYMOUS, -1, (uintptr_t)INT_CTRL_BASE);

	priority(1);

	interrupt(IRQ_UNUSED, irqHandler, NULL, 0, NULL);

	for (int i = 0; i < BENCHMARKS; ++i) {
		benchEnd = 0;
		uint64_t benchStart = getCntr();
		/* Force interrupt on CPU 0 */
		*(irqCtrl + 2) = (1 << IRQ_UNUSED);

		while (benchEnd == 0) {
			__asm__ volatile("nop");
		}

		results[i] = benchEnd - benchStart;
	}

	for (int i = 0; i < BENCHMARKS; i++) {
		printf("%llu%c", results[i], (i == BENCHMARKS - 1) ? '\n' : ',');
	}

	return 0;
}
