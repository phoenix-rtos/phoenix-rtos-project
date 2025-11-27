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
	char stacks[NUM_THREADS][1024];
} common = { 0 };


void init(void)
{
	oid_t oid;

	while (lookup(DEV_PATH, NULL, &oid) < 0) {
		usleep(100 * 1000);
	}

	common.srv_port = oid.port;
}


void another(void *arg)
{
	doClientLoop(THREAD_PRIO, msgSend, common.srv_port, common.data, NULL);

	fprintf(stderr, "DONE\n");

	return;
}


int main(void)
{
	init();
	fprintf(stderr, "init ok\n");

	trace_ctx_t ctx;
	startTrace(&ctx);

	// for (size_t i = 0; i < NUM_THREADS; i++) {
	// 	int res = beginthread(another, THREAD_PRIO, &common.stacks[i], sizeof(common.stacks[i]), NULL);
	// 	if (res < 0) {
	// 		fprintf(stderr, "failed to beginthread: %d", res);
	// 		return 1;
	// 	}
	// }


	for (size_t prio = 2; prio < MAX_PRIO + 1; prio++) {
		doClientLoop(prio, msgSend, common.srv_port, common.data, NULL);
	}

	stopTrace(&ctx);

	return 0;
}
