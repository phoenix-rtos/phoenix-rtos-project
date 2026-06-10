/*
 * Phoenix-RTOS
 *
 * IPC benchmark - main entry point
 *
 * Standalone IPC benchmark application.  Measures raw msgSend round-trip
 * latency at various server-chain depths and payload sizes, and POSIX-level
 * IPC latency (open, close, read, write, stat, lookup).
 *
 * Copyright 2026 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"


static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [-c]\n", prog);
	fprintf(stderr, "  -c  print CSV output to stdout\n");
}


int main(int argc, char *argv[])
{
	int csv = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			csv = 1;
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
		else {
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	fprintf(stderr, "ipcbench: starting IPC benchmarks\n");

	int ret = bench_run_all(csv);

	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
