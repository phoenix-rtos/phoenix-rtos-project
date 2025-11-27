#ifndef _MSGMARK_COMMON_H_
#define _MSGMARK_COMMON_H_


#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/threads.h>
#include <sys/msg.h>
#include <trace.h>
#include <sys/mman.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>


#define DEV_PATH          "/dev/msgmark"
#define DEV_FASTPATH_PATH "/dev/msgmark_fastpath"

#define USE_SHM 1


#define US_BETWEEN(ts0, ts1) \
	((ts1).tv_sec - (ts0).tv_sec) * 1000000 + ((ts1).tv_nsec - (ts0).tv_nsec) / 1000;


static inline void cpuid(void)
{
	/* results ignored, used only for clearing the pipeline */
	__asm__ __volatile__("cpuid" : :
			: "eax", "ebx", "ecx", "edx");
}

#define RETRIES 10000

#define VERBOSE 0

#define BUF_SIZE      (200 << 10) /* 2000 KiB */
#define CHUNK_SIZE    (BUF_SIZE / 16)
#define PERF_BUF_SIZE (512 << 10)

#define NUM_THREADS 4
#define THREAD_PRIO 4

#define MAX_PRIO 2

#if 1

#define UNIT "us"


static inline void *initPoints(size_t n)
{
	return malloc(sizeof(struct timespec) * n);
}


static inline void measurePoint(void *priv, size_t i)
{
	struct timespec *ts = priv;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[i]);
}


static inline uint64_t delta(void *priv, size_t i1, size_t i2)
{
	struct timespec *ts = priv;
	return US_BETWEEN(ts[0], ts[1]);
}

#else


#define UNIT "cycles"


static inline void *initPoints(size_t n)
{
	return malloc(sizeof(uint64_t) * n);
}


static inline uint64_t rdtsc(void)
{
	unsigned int lo, hi;

	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));

	return ((uint64_t)hi << 32) | lo;
}


static inline void measurePoint(void *priv, size_t i)
{
	uint64_t *cs = priv;
	cs[i] = rdtsc();
}


static inline uint64_t delta(void *priv, size_t i1, size_t i2)
{
	uint64_t *cs = priv;
	return cs[i2] - cs[i1];
}


#endif


static inline void doClientLoop(size_t prio, int (*syscall)(uint32_t port, msg_t *msg), uint32_t port, char *buf, char *shmBuf)
{
	int res;
	void *p = initPoints(2);
	if (p == NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	priority(prio);

	uint64_t rtt = 0;
	msg_t msg = { 0 };

	for (size_t size = 0; size < BUF_SIZE; size += CHUNK_SIZE) {
		for (size_t i = 0; i < RETRIES; i++) {
			measurePoint(p, 0);
			if (shmBuf == NULL) {
				msg.i.size = size;
				msg.i.data = buf;
			}
			else {
				memcpy(shmBuf, buf, size);
			}
			res = syscall(port, &msg);
			if (res < 0) {
				fprintf(stderr, "msgCall failed: %d\n", res);
			}
			measurePoint(p, 1);
#if VERBOSE
			fprintf(stderr, "msgCall\n");
#endif

			rtt += delta(p, 0, 1);
		}

		rtt /= RETRIES;

		fprintf(stderr, "%zu, %zu, %ju\n", prio, size, (uintmax_t)rtt);
		// fprintf(stderr, "prio=%zu size=%zu rtt=%ju " UNIT "\n", prio, size, (uintmax_t)rtt);
	}
}


#define TRACE 0

#define PERF_BUF_SIZE (512 << 10)
#define TRACE_DIR     "/msg_trace"


static inline void startTrace(trace_ctx_t *ctx)
{
	if (TRACE) {
		int res = trace_init(ctx, false);
		if (res < 0) {
			fprintf(stderr, "trace_init failed: %d\n", res);
			exit(1);
		}

		res = trace_start(ctx);
		if (res < 0) {
			fprintf(stderr, "trace_start failed: %d\n", res);
			exit(1);
		}
	}
}


static inline void stopTrace(trace_ctx_t *ctx)
{
	if (TRACE) {
		int res = trace_stopAndGather(ctx, PERF_BUF_SIZE, TRACE_DIR);
		if (res < 0) {
			fprintf(stderr, "trace_stopAndGather failed: %d\n", res);
			exit(1);
		}
	}
}


static inline char *initShm(bool creat)
{
	int ret;
	int flags = O_RDWR;
	if (creat) {
		flags |= O_CREAT;
	}

	int fd = open("/tmp/shm", flags);
	if (fd < 0) {
		fprintf(stderr, "open failed: %d\n", errno);
		return NULL;
	}

	ret = ftruncate(fd, BUF_SIZE);
	if (ret < 0) {
		fprintf(stderr, "ftruncate failed: %d\n", fd);
		return NULL;
	}

	struct stat s;
	ret = fstat(fd, &s);
	if (ret < 0) {
		fprintf(stderr, "%d fail: %d\n", __LINE__, ret);
		return NULL;
	}

	size_t fileSize = s.st_size;
	fprintf(stderr, "size=%zu\n", fileSize);

	assert(fileSize == BUF_SIZE);

	char *v = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (creat) {
		fprintf(stderr, "pre %d\n", *v);
		*v = 42;
		fprintf(stderr, "post %d\n", *v);
	}

	if (v == MAP_FAILED) {
		fprintf(stderr, "mmap failed\n");
		return NULL;
	}

	return v;
}


#endif /* _MSGMARK_COMMON_H_ */
