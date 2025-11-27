#include <sys/threads.h>
#include <sys/msg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <posix/utils.h>
#include <time.h>

#include "common.h"


int main(void)
{
	uint32_t port;
	oid_t oid;
	msg_t msg = { 0 };
	msg_rid_t rid;

	if (portCreate(&port) != 0) {
		fprintf(stderr, "portCreate");
		return 1;
	}

	oid.port = port;
	oid.id = 0;
	if (create_dev(&oid, DEV_PATH) != 0) {
		fprintf(stderr, "create_dev");
		return 1;
	}

	priority(0);

	while (1) {
		msgRecv(port, &msg, &rid);
		// clock_gettime(CLOCK_MONOTONIC_RAW, (struct timespec *)&msg.o.raw);
		// msg.o.size = sizeof(struct timespec);
		msgRespond(port, &msg, rid);
	}

	return 0;
}
