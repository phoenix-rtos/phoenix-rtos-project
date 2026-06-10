/*
 * Phoenix-RTOS
 *
 * IPC benchmark - benchmark engine
 *
 * Copyright 2026 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _IPCBENCH_BENCH_H_
#define _IPCBENCH_BENCH_H_

#include <stddef.h>


/* Run all benchmarks and print results to stderr.
 * If csv is non-zero, also prints CSV to stdout.
 * Returns 0 on success, negative on error. */
int bench_run_all(int csv);


#endif /* _IPCBENCH_BENCH_H_ */
