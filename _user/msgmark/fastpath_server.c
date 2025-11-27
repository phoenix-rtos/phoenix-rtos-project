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
	fprintf(stderr, "main: %p\n", main);

	uint32_t port;
	oid_t oid;
	msg_t msg = { 0 };
	msg_rid_t rid;

	void *utcb = msgConfigure();
	if (utcb == NULL) {
		fprintf(stderr, "server msgConfigure failed\n");
	}

	fprintf(stderr, "server msg=%p utcb=%p\n", (void *)&msg, utcb);

	ipc_buf_t *utcbMsg = (ipc_buf_t *)utcb;
	fprintf(stderr, "writing\n");
	utcbMsg->raw[0] = 111;
	utcbMsg->size = 1;
	fprintf(stderr, "done\n");

	char *shmBuf = NULL;

#if USE_SHM
	shmBuf = initShm(true);
	if (shmBuf == NULL) {
		fprintf(stderr, "initShm failed\n");
		return 1;
	}
#endif

	if (portCreate(&port) != 0) {
		fprintf(stderr, "portCreate");
		return 1;
	}

	oid.port = port;
	oid.id = 0;
	if (create_dev(&oid, DEV_FASTPATH_PATH) != 0) {
		fprintf(stderr, "create_dev");
		return 1;
	}

	priority(1);

	rid = -1;

	size_t count = 0;
	while (1) {
		msgRespondAndRecv(port, &msg, &rid);
#if VERBOSE
		fprintf(stderr, "OK %zu (buf type=%d)\n", count, utcbMsg->type);
#endif
		count++;
		// if (utcbMsg->type == 42) {
		// 	fprintf(stderr, "sleep");
		// 	usleep(100 * 1000);
		// 	fprintf(stderr, "back");
		// }
	}

	return 0;
}
