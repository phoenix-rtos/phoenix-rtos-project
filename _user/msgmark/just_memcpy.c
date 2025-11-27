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


struct {
	uint32_t srv_port;
	char data[BUF_SIZE];
	char dest[BUF_SIZE];
	char stacks[NUM_THREADS][1024];
} common = { 0 };


int main(void)
{
	void *p = initPoints(2);
	if (p == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	size_t prio = 2;

	priority(prio);
	uint64_t rtt = 0;

	for (size_t i = 0; i < BUF_SIZE; i += 128) {
		common.data[i] = i;
		common.dest[i] = i - 1;
	}

	for (size_t size = 0; size < BUF_SIZE; size += CHUNK_SIZE) {
		for (size_t i = 0; i < RETRIES; i++) {
			measurePoint(p, 0);
			memcpy(common.dest, common.data, size);
			measurePoint(p, 1);

			rtt += delta(p, 0, 1);
		}

		rtt /= RETRIES;

		fprintf(stderr, "%zu, %zu, %ju\n", prio, size, (uintmax_t)rtt);
	}

	return 0;
}
