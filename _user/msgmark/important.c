#include <string.h>
#include <sys/threads.h>
#include <sys/msg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <posix/utils.h>
#include <time.h>

#include <trace.h>

#include "common.h"


int main(void)
{
	struct timespec ts[2];

	priority(1);

	size_t us = 100 * 1000, elapsed = 0;

	for (;;) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &ts[0]);
		usleep(1000);
		clock_gettime(CLOCK_MONOTONIC_RAW, &ts[1]);

		elapsed = US_BETWEEN(ts[0], ts[1]);
		if (10 * elapsed > 11 * us) {
			fprintf(stderr, "late! %zu us\n", elapsed - us);
		}
	}

	return 0;
}
