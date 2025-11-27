#include <stdlib.h>
#include <string.h>
#include <sys/threads.h>
#include <sys/msg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <posix/utils.h>
#include <time.h>


#define BUF_SIZE (200 << 10) /* 2000 KiB */


struct {
	char data[BUF_SIZE];
} common = { 0 };


#define RETRIES 10000

#define BUF_SIZE   (200 << 10) /* 2000 KiB */
#define CHUNK_SIZE (BUF_SIZE / 16)

#define US_BETWEEN(ts0, ts1) \
	((ts1).tv_sec - (ts0).tv_sec) * 1000000 + ((ts1).tv_nsec - (ts0).tv_nsec) / 1000;


int main(int argc, char **argv)
{
	uint32_t inPort = 0;
	uint32_t outPort = 0;

	void *utcb = msgConfigure();
	if (utcb == NULL) {
		fprintf(stderr, "server msgConfigure failed\n");
	}

	char path[128];

	if (argc != 3) {
		fprintf(stderr, "usage: %s IN_ID OUT_ID\n", argv[0]);
		return 1;
	}

	uint32_t inId = strtoul(argv[1], NULL, 10);
	uint32_t outId = strtoul(argv[2], NULL, 10);

	ipc_buf_t *ipcBuf = (ipc_buf_t *)utcb;
	uint32_t *ipcU32Buf = (uint32_t *)ipcBuf->raw;

	if (inId == 0 && outId == 0) {
		fprintf(stderr, "at least one id must be non-zero\n");
		return 1;
	}

	if (inId != 0) {
		snprintf(path, sizeof(path), "/dev/msgcor%d", inId);

		oid_t oid;
		while (lookup(path, NULL, &oid) < 0) {
			usleep(100 * 1000);
		}
		inPort = oid.port;
	}

	if (outId != 0) {
		if (portCreate(&outPort) != 0) {
			fprintf(stderr, "portCreate");
			return 1;
		}

		snprintf(path, sizeof(path), "/dev/msgcor%d", outId);

		oid_t oid;

		oid.port = outPort;
		oid.id = 0;
		if (create_dev(&oid, path) != 0) {
			fprintf(stderr, "create_dev");
			return 1;
		}
	}

	fprintf(stderr, "(%u, %u) init ok\n", inId, outId);

	msg_t msg = { 0 };

	struct timespec ts[2];
	uint64_t rtt = 0;
	uint64_t ms;
	size_t prio = 3;

	priority(prio);

	uint32_t depth = 0;
	uint32_t maxDepth = 0;

	size_t size = 0;

	ipcBuf->size = 2 * sizeof(uint32_t);

	/* TODO: if something exits on chain there is a pagefault
		obviously the error paths are not unhandled yet, but this case is worth remembering */

	for (size_t i = 0; i < RETRIES; i++) {
		msg_rid_t rid;
		if (inId != 0) {
			// msgRecv(inPort, &msg, &rid);
			ipcU32Buf[0] = depth - 1;
			msgRespondAndRecv(inPort, &msg, &rid);
			if (ipcBuf->size != 2 * sizeof(uint32_t)) {
				fprintf(stderr, "bad size: %zu\n", ipcBuf->size);
				exit(1);
			}
			depth = ipcU32Buf[0];
		}

		if (outId != 0) {
			msg.i.size = size;
			msg.i.data = &common.data;

			ipcU32Buf[0] = ipcU32Buf[1] = depth + 1;
			clock_gettime(CLOCK_MONOTONIC_RAW, &ts[0]);
			msgCall(outPort, &msg);
			clock_gettime(CLOCK_MONOTONIC_RAW, &ts[1]);

			if (ipcBuf->size != 2 * sizeof(uint32_t)) {
				fprintf(stderr, "bad size: %zu\n", ipcBuf->size);
				exit(1);
			}
			if (ipcU32Buf[0] != depth) {
				fprintf(stderr, "bad depth: %u\n", ipcU32Buf[0]);
				exit(1);
			}

			maxDepth = ipcU32Buf[1];

			ms = US_BETWEEN(ts[0], ts[1]);
			if (inId == 0) {
				fprintf(stderr, "%u %u i=%zu %llu ms\n", depth, maxDepth, i, ms);
			}

			rtt += ms;
		}

		if (inId != 0) {
			// msgRespond(inPort, &msg, rid);
		}
	}

	rtt /= RETRIES;

	if (inId == 0) {
		fprintf(stderr, "(%u, %u) exiting - prio=%zu size=%zu rtt=%ju ms\n", inId, outId, prio, size, (uintmax_t)rtt);
	}

	portDestroy(outPort);

	return 0;
}
