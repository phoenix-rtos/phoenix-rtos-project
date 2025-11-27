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

	while (lookup(DEV_FASTPATH_PATH, NULL, &oid) < 0) {
		usleep(100 * 1000);
	}

	common.srv_port = oid.port;
}


void another(void *arg)
{
	void *utcb = msgConfigure();
	if (utcb == NULL) {
		fprintf(stderr, "client msgConfigure failed\n");
		return;
	}
	ipc_buf_t *utcbMsg = (ipc_buf_t *)utcb;
	utcbMsg->raw[0] = 77;
	utcbMsg->size = 1;

	doClientLoop(THREAD_PRIO, msgCall, common.srv_port, common.data, arg);

	fprintf(stderr, "DONE\n");

	return;
}


int main(void)
{
	fprintf(stderr, "main: %p\n", main);

	init();

	void *utcb = msgConfigure();
	if (utcb == NULL) {
		fprintf(stderr, "client msgConfigure failed\n");
		exit(1);
	}

	fprintf(stderr, "init ok\n");

	fprintf(stderr, "client utcb=%p\n", utcb);

	msg_t *utcbMsg = (msg_t *)utcb;
	utcbMsg->type = 42;

	char *shmBuf = NULL;

#if USE_SHM
	shmBuf = initShm(false);
	if (shmBuf == NULL) {
		fprintf(stderr, "initShm failed\n");
		return 1;
	}
#endif

	trace_ctx_t ctx;

	startTrace(&ctx);

	// for (size_t i = 0; i < NUM_THREADS; i++) {
	// 	int res = beginthread(another, THREAD_PRIO, &common.stacks[i], sizeof(common.stacks[i]), shmBuf);
	// 	if (res < 0) {
	// 		fprintf(stderr, "failed to beginthread: %d", res);
	// 		return 1;
	// 	}
	// }

	for (size_t prio = 2; prio < MAX_PRIO + 1; prio++) {
		doClientLoop(prio, msgCall, common.srv_port, common.data, shmBuf);
	}

	stopTrace(&ctx);

	fprintf(stderr, "MAIN THREAD DONE\n");

	return 0;
}
