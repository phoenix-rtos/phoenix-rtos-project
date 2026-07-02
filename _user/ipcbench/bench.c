/*
 * Phoenix-RTOS
 *
 * IPC benchmark - benchmark engine
 *
 * Benchmarks raw IPC (msgSend round-trips at various depths and payload
 * sizes) and POSIX-level IPC (open, close, read, write, stat, lookup,
 * create, unlink, link).
 *
 * Copyright 2026 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/threads.h>
#include <sys/wait.h>
#include <sys/debug.h>
#include <posix/utils.h>

#include "server.h"
#include "bench.h"


#define MAX_CHAIN_DEPTH 5
#define WARMUP_ITERS    10
#define LATENCY_ITERS   500


static int csv_output;


/* ---- timing helpers ---- */


static time_t gettime_us(void)
{
	time_t t;
	gettime(&t, NULL);
	return t;
}


static void report(const char *name, int iterations, time_t total_us)
{
	time_t avg = (iterations > 0) ? total_us / iterations : 0;
	fprintf(stderr, "  %-42s %6d iters  %8llu us total  %6llu us/iter\n",
			name, iterations, (unsigned long long)total_us, (unsigned long long)avg);

	if (csv_output != 0) {
		printf("%s,%d,%llu,%llu,,,,,\n",
				name, iterations, (unsigned long long)total_us, (unsigned long long)avg);
	}
}


static int time_cmp(const void *a, const void *b)
{
	time_t ta = *(const time_t *)a;
	time_t tb = *(const time_t *)b;

	if (ta < tb) {
		return -1;
	}
	if (ta > tb) {
		return 1;
	}
	return 0;
}


static void report_latency(const char *name, time_t *samples, int n)
{
	if (n <= 0) {
		return;
	}

	qsort(samples, (size_t)n, sizeof(time_t), time_cmp);

	time_t min_v = samples[0];
	time_t max_v = samples[n - 1];
	time_t med = samples[n / 2];
	time_t p99 = samples[(n * 99) / 100];

	time_t sum = 0;
	for (int i = 0; i < n; i++) {
		sum += samples[i];
	}
	time_t avg = sum / n;
	time_t jitter = max_v - min_v;

	fprintf(stderr, "  %-42s %6d iters  min=%llu med=%llu avg=%llu p99=%llu max=%llu jitter=%llu us\n",
			name, n,
			(unsigned long long)min_v, (unsigned long long)med,
			(unsigned long long)avg, (unsigned long long)p99,
			(unsigned long long)max_v, (unsigned long long)jitter);

	if (csv_output != 0) {
		printf("%s,%d,,%llu,%llu,%llu,%llu,%llu,%llu\n",
				name, n,
				(unsigned long long)avg,
				(unsigned long long)min_v, (unsigned long long)med,
				(unsigned long long)p99, (unsigned long long)max_v,
				(unsigned long long)jitter);
	}
}


/* ---- device path generation ---- */


static int path_counter = 0;


static void make_path(char *buf, size_t sz, const char *tag)
{
	snprintf(buf, sz, "/dev/ib_%s_%d_%d", tag, getpid(), path_counter++);
}


/* ---- wait for server path to appear ---- */


static int wait_for_dev(const char *path, oid_t *oid, int timeout_ms)
{
	int elapsed = 0;

	while (lookup(path, NULL, oid) < 0) {
		if (elapsed >= timeout_ms) {
			return -1;
		}
		usleep(10 * 1000);
		elapsed += 10;
	}

	return 0;
}


/* ================================================================
 * Mandatory cost benchmakrs
 * ================================================================ */


static int bench_mandatory_syscall_enter_exit(int iterations)
{
	time_t start, end;

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		(void)bench_entry_exit_time();
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		(void)bench_entry_exit_time();
	}

	end = gettime_us();
	report("mandatory entry+exit", iterations, end - start);

	return 0;
}


/* ================================================================
 * Raw IPC benchmarks (msgSend round-trip)
 * ================================================================ */


/* Spawn a chain of depth servers: chain[0] = echo leaf, chain[i>0] = forwarder.
 * Returns number of PIDs stored in out_pids, or -1 on error. */
static int spawn_chain(int depth, int use_rr, oid_t *front_oid, pid_t *out_pids)
{
	char paths[MAX_CHAIN_DEPTH][64];
	int n = 0;

	if (depth < 1 || depth > MAX_CHAIN_DEPTH) {
		return -1;
	}

	for (int i = 0; i < depth; i++) {
		make_path(paths[i], sizeof(paths[i]), "raw");
	}

	/* Leaf echo server */
	out_pids[n] = server_spawn_echo(paths[0], use_rr, 4);
	if (out_pids[n] < 0) {
		return -1;
	}
	n++;

	/* Forwarding servers */
	for (int i = 1; i < depth; i++) {
		oid_t backend;
		if (wait_for_dev(paths[i - 1], &backend, 5000) < 0) {
			return -1;
		}

		out_pids[n] = server_spawn_forwarder(paths[i], backend.port);
		if (out_pids[n] < 0) {
			return -1;
		}
		n++;
	}

	if (wait_for_dev(paths[depth - 1], front_oid, 5000) < 0) {
		return -1;
	}

	return n;
}


static void kill_chain(pid_t *pids, int n)
{
	for (int i = n - 1; i >= 0; i--) {
		server_kill(pids[i]);
	}
}


static int bench_raw_ipc(int depth, size_t payload_sz, int iterations, int use_rr,
		int direction)
{
	oid_t oid;
	pid_t pids[MAX_CHAIN_DEPTH];
	int npids;
	char *buf = NULL;
	char label[64];
	time_t start, end;
	const char *dir_str;

	npids = spawn_chain(depth, use_rr, &oid, pids);
	if (npids <= 0) {
		fprintf(stderr, "  ERROR: failed to spawn chain depth=%d\n", depth);
		return -1;
	}

	if (payload_sz > 0) {
#ifndef IPCBENCH_NO_EXT_BUF
		if (payload_sz <= 2 * _PAGE_SIZE) {
			buf = msgSetup(2 * _PAGE_SIZE);
			if (buf == NULL) {
				fprintf(stderr, "  ERROR: msgSetup failed\n");
			}
		}
		else {
#endif
			buf = malloc(payload_sz);
#ifndef IPCBENCH_NO_EXT_BUF
		}
#endif

		if (buf == NULL) {
			kill_chain(pids, npids);
			return -1;
		}
		memset(buf, 0xAB, payload_sz);
	}

	switch (direction) {
		case 0: dir_str = "rd"; break;    /* server -> client (o.data only) */
		case 1: dir_str = "wr"; break;    /* client -> server (i.data only) */
		default: dir_str = "bidi"; break; /* both directions */
	}

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		if (direction == 0 || direction == 2) {
			msg.o.size = payload_sz;
			msg.o.data = buf;
		}
		if (direction == 1 || direction == 2) {
			msg.i.size = payload_sz;
			msg.i.data = buf;
		}
		msgSend(oid.port, &msg);
	}

	/* Timed loop */
	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		if (direction == 0 || direction == 2) {
			msg.o.size = payload_sz;
			msg.o.data = buf;
		}
		if (direction == 1 || direction == 2) {
			msg.i.size = payload_sz;
			msg.i.data = buf;
		}

		if (msgSend(oid.port, &msg) != 0) {
			fprintf(stderr, "  ERROR: msgSend failed at iter %d\n", i);
			break;
		}
	}

	end = gettime_us();

	snprintf(label, sizeof(label), "raw %s d=%d p=%zu rr=%d",
			dir_str, depth, payload_sz, use_rr);
	report(label, iterations, end - start);

#ifndef IPCBENCH_NO_EXT_BUF
	if (payload_sz > 2 * _PAGE_SIZE)
#endif
		free(buf);
	kill_chain(pids, npids);

	return 0;
}


/* ================================================================
 * POSIX-level IPC benchmarks
 * ================================================================ */


static pid_t posix_srv_pid;
static char posix_dev_path[64];


static int posix_setup(void)
{
	make_path(posix_dev_path, sizeof(posix_dev_path), "posix");

	posix_srv_pid = server_spawn_posix(posix_dev_path);
	if (posix_srv_pid < 0) {
		return -1;
	}

	/* Wait for server to appear */
	oid_t oid;
	if (wait_for_dev(posix_dev_path, &oid, 5000) < 0) {
		server_kill(posix_srv_pid);
		return -1;
	}

	return 0;
}


static void posix_teardown(void)
{
	server_kill(posix_srv_pid);
}


static int bench_posix_open_close(int iterations)
{
	time_t start, end;

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		int fd = open(posix_dev_path, O_RDWR);
		if (fd >= 0) {
			close(fd);
		}
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		int fd = open(posix_dev_path, O_RDWR);
		if (fd < 0) {
			fprintf(stderr, "  ERROR: open failed: %s\n", strerror(errno));
			return -1;
		}
		close(fd);
	}

	end = gettime_us();
	report("posix open+close", iterations, end - start);

	return 0;
}


static int bench_posix_read(size_t sz, int iterations)
{
	char label[64];
	char *buf;
	time_t start, end;

	int fd = open(posix_dev_path, O_RDWR);
	if (fd < 0) {
		return -1;
	}

	buf = malloc(sz > 0 ? sz : 1);
	if (buf == NULL) {
		close(fd);
		return -1;
	}

	/* Pre-fill server file with data via write */
	if (sz > 0) {
		memset(buf, 'R', sz);
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, sz);
	}

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		lseek(fd, 0, SEEK_SET);
		read(fd, buf, sz);
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		lseek(fd, 0, SEEK_SET);
		if (read(fd, buf, sz) < 0) {
			fprintf(stderr, "  ERROR: read failed\n");
			break;
		}
	}

	end = gettime_us();

	snprintf(label, sizeof(label), "posix read sz=%zu", sz);
	report(label, iterations, end - start);

	free(buf);
	close(fd);
	return 0;
}


static int bench_posix_write(size_t sz, int iterations)
{
	char label[64];
	char *buf;
	time_t start, end;

	int fd = open(posix_dev_path, O_RDWR);
	if (fd < 0) {
		return -1;
	}

	buf = malloc(sz > 0 ? sz : 1);
	if (buf == NULL) {
		close(fd);
		return -1;
	}
	memset(buf, 'W', sz);

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, sz);
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		lseek(fd, 0, SEEK_SET);
		if (write(fd, buf, sz) < 0) {
			fprintf(stderr, "  ERROR: write failed\n");
			break;
		}
	}

	end = gettime_us();

	snprintf(label, sizeof(label), "posix write sz=%zu", sz);
	report(label, iterations, end - start);

	free(buf);
	close(fd);
	return 0;
}


static int bench_posix_stat(int iterations)
{
	struct stat st;
	time_t start, end;

	int fd = open(posix_dev_path, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		fstat(fd, &st);
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		if (fstat(fd, &st) < 0) {
			fprintf(stderr, "  ERROR: fstat failed\n");
			break;
		}
	}

	end = gettime_us();
	report("posix fstat", iterations, end - start);

	close(fd);
	return 0;
}


/* ================================================================
 * Name resolution benchmarks (proc_portLookup / proc_lookup path)
 * ================================================================ */


static int bench_lookup_cached(int iterations)
{
	oid_t oid;
	time_t start, end;

	/* Warmup - ensure path is in dcache */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		lookup(posix_dev_path, NULL, &oid);
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		if (lookup(posix_dev_path, NULL, &oid) < 0) {
			fprintf(stderr, "  ERROR: lookup failed\n");
			break;
		}
	}

	end = gettime_us();
	report("lookup cached (dcache hit)", iterations, end - start);

	return 0;
}


static int bench_lookup_root(int iterations)
{
	oid_t oid;
	time_t start, end;

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		lookup("/", NULL, &oid);
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		if (lookup("/", NULL, &oid) < 0) {
			fprintf(stderr, "  ERROR: lookup / failed\n");
			break;
		}
	}

	end = gettime_us();
	report("lookup root (/)", iterations, end - start);

	return 0;
}


static int bench_portcreate_destroy(int iterations)
{
	time_t start, end;

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		uint32_t p;
		if (portCreate(&p) == 0) {
			portDestroy(p);
		}
	}

	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		uint32_t p;
		if (portCreate(&p) < 0) {
			fprintf(stderr, "  ERROR: portCreate failed\n");
			break;
		}
		portDestroy(p);
	}

	end = gettime_us();
	report("portCreate+portDestroy", iterations, end - start);

	return 0;
}


static int bench_portregister_unregister(int iterations)
{
	time_t start, end;
	uint32_t port;
	char path[64];

	if (portCreate(&port) < 0) {
		return -1;
	}

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		snprintf(path, sizeof(path), "/dev/ib_reg_warmup_%d_%d", getpid(), i);
		oid_t oid = { .port = port, .id = 0 };
		create_dev(&oid, path);
		/* Note: there's no unregister via POSIX, but portDestroy cleans up */
	}

	portDestroy(port);

	/* For actual timing, create fresh port each iteration */
	start = gettime_us();

	for (int i = 0; i < iterations; i++) {
		uint32_t p;
		if (portCreate(&p) < 0) {
			break;
		}
		oid_t oid = { .port = p, .id = 0 };
		snprintf(path, sizeof(path), "/dev/ib_reg_%d_%d", getpid(), i);
		create_dev(&oid, path);
		portDestroy(p);
	}

	end = gettime_us();
	report("portCreate+register+destroy", iterations, end - start);

	return 0;
}


/* ================================================================
 * Raw IPC benchmark table
 * ================================================================ */


struct raw_bench_spec {
	size_t payload;
	int iterations;
	int use_rr;
	int direction; /* 0=read(o.data), 1=write(i.data), 2=bidi */
};


static const int raw_depths[] = { 1, 3 };


static const struct raw_bench_spec raw_specs[] = {
/* Zero payload (no buffers mapped, pure IPC overhead) */
#if 1
	{ 0, 1000, 0, 0 },
	{ 0, 500, 0, 0 },
#ifndef IPCBENCH_NO_RR
	{ 0, 1000, 1, 0 },
	{ 0, 500, 1, 0 },
#endif

	/* Small payload read direction (64 bytes, single buffer map) */
	{ 64, 1000, 0, 0 },
	{ 64, 500, 0, 0 },
#ifndef IPCBENCH_NO_RR
	{ 64, 1000, 1, 0 },
	{ 64, 500, 1, 0 },
#endif

	/* Small payload write direction */
	{ 64, 1000, 0, 1 },
	{ 64, 500, 0, 1 },
#ifndef IPCBENCH_NO_RR
	{ 64, 1000, 1, 1 },
	{ 64, 500, 1, 1 },
#endif

	/* Medium payload read direction */
	{ 2048, 1000, 0, 0 },
	{ 2048, 500, 0, 0 },
#ifndef IPCBENCH_NO_RR
	{ 2048, 1000, 1, 0 },
	{ 2048, 500, 1, 0 },
#endif

	/* Medium payload write direction */
	{ 2048, 1000, 0, 1 },
	{ 2048, 500, 0, 1 },
#ifndef IPCBENCH_NO_RR
	{ 2048, 1000, 1, 1 },
	{ 2048, 500, 1, 1 },
#endif

	/* Large (full-page) payload read direction */
	/* TODO: stress the alignment */
	{ 4096, 500, 0, 0 },
	{ 4096, 200, 0, 0 },
#ifndef IPCBENCH_NO_RR
	{ 4096, 500, 1, 0 },
	{ 4096, 200, 1, 0 },
#endif

	/* Large SHM payload write direction */
	{ 4096, 1000, 0, 1 },
	{ 4096, 500, 0, 1 },
#ifndef IPCBENCH_NO_RR
	{ 4096, 1000, 1, 1 },
	{ 4096, 500, 1, 1 },
#endif

	/* Large SHM payload bidirectional (both buffers mapped) */
	{ 4096, 500, 0, 2 },

	/* Large+ (over-page) payload read/write/bidi */
	{ 4096 + 8, 500, 0, 0 },
	{ 4096 + 8, 1000, 0, 1 },
	{ 4096 + 8, 500, 0, 2 },

	/* Very large SHM payload (32KB) */
	{ 32768, 200, 0, 0 },
	{ 32768, 200, 0, 1 },
	{ 32768, 200, 0, 2 },
	{ 32768, 100, 0, 0 },
#else
	{ 64, 500, 0, 0 },
	{ 64, 500, 1, 0 },
	{ 2048, 500, 0, 0 },
	{ 2048, 500, 1, 0 },
#endif
};


/* ================================================================
 * POSIX-level benchmark table
 * ================================================================ */


struct posix_rw_spec {
	size_t size;
	int iterations;
};


static const struct posix_rw_spec posix_rw_specs[] = {
	{ 0, 1000 },
	{ 64, 1000 },
	{ 4096, 500 },
	{ 32768, 200 },
};


/* ================================================================
 * Real-time / priority inversion benchmarks
 *
 * These test non-trivial interactions between IPC priority donation
 * (scheduling context / SC) and mutex priority inheritance (PI).
 * Each benchmark measures high-priority IPC round-trip latency under
 * conditions that will reveal unbounded priority inversion if the
 * kernel doesn't correctly propagate priorities through IPC+mutex chains.
 *
 * Priority numbering: lower number = higher priority (P1 > P2 > ... > P7).
 * ================================================================ */


#define RT_ITERS    200
#define RT_P_HIGH   1  /* highest prio client */
#define RT_P_SERVER 3  /* server thread base prio */
#define RT_P_MEDIUM 8  /* interferer: high enough to preempt un-boosted */
#define RT_P_LOW    10 /* lock holder base prio */
#define RT_P_LOWEST 12 /* backend server base prio */

#define RT_BUSY_ITERS 20000000 /* ~200ms busy loop on QEMU riscv64 */


/*
 * Server that acquires a mutex before responding to IPC.
 * Used by the three-task and cascaded benchmarks.
 *
 * Protocol:
 *   1. Creates port, registers dev, writes port to pipe
 *   2. Spawns holder thread (grabs mutex)
 *   3. Spawns interferer thread (CPU hog at medium prio)
 *   4. Receives IPC (gets SC donation), blocks on mutex
 *   5. After acquiring mutex, responds to IPC
 */

struct rt_server_args {
	int pipe_fd; /* write end: server signals readiness */
	const char *dev_path;
	int holder_prio;
	int medium_prio;
	int holder_work_us; /* how long holder holds the lock (us) */
	int spawn_medium;   /* whether to spawn medium-prio interferer */
};


/* Cross-thread shared state inside the forked server */
static struct {
	handle_t mutex;
	volatile int phase;
	volatile int round;
	char stack[3][4096] __attribute__((aligned(8)));
} rt_srv;


static void rt_holder_thread(void *arg)
{
	int hold_us = (int)(intptr_t)arg;
	int my_round = 0;

	for (;;) {
		mutexLock(rt_srv.mutex);
		rt_srv.phase = 1; /* mutex held */

		/* Wait for the server thread to have received IPC and be about to
		 * contend on the mutex (so the PI chain forms properly) */
		while (rt_srv.phase < 2) {
			usleep(1000);
		}
		usleep(20 * 1000); /* ensure server thread actually blocks */

		/* Simulate critical section work */
		if (hold_us > 0) {
			usleep(hold_us);
		}

		mutexUnlock(rt_srv.mutex);

		/* Wait for server to finish this round before re-locking */
		my_round++;
		while (rt_srv.round < my_round) {
			usleep(1000);
		}
	}
}


static void rt_medium_thread(void *arg)
{
	int my_round = 0;

	for (;;) {
		/* CPU-bound interferer: will cause unbounded inversion if the lock
		 * holder is not properly boosted above this thread's priority */
		while (rt_srv.phase < 2) {
			usleep(1000);
		}
		usleep(10 * 1000); /* let the PI chain form */

		for (volatile int i = 0; i < RT_BUSY_ITERS && rt_srv.phase != 0; i++) {
			;
		}

		/* Wait for next round */
		my_round++;
		while (rt_srv.round < my_round) {
			usleep(1000);
		}
	}
}


static void rt_server_process(struct rt_server_args *args)
{
	uint32_t port;
	oid_t oid;
	msg_t msg = { 0 };
	msg_rid_t rid;

	if (portCreate(&port) < 0) {
		_exit(3);
	}
	oid.port = port;
	oid.id = 0;
	if (create_dev(&oid, args->dev_path) < 0) {
		_exit(3);
	}

	mutexCreate(&rt_srv.mutex);
	rt_srv.phase = 0;
	rt_srv.round = 0;

	/* Spawn long-lived lock holder thread */
	beginthread(rt_holder_thread, args->holder_prio,
			rt_srv.stack[0], sizeof(rt_srv.stack[0]),
			(void *)(intptr_t)args->holder_work_us);

	/* Optionally spawn long-lived medium-prio interferer */
	if (args->spawn_medium != 0) {
		beginthread(rt_medium_thread, args->medium_prio,
				rt_srv.stack[1], sizeof(rt_srv.stack[1]), NULL);
	}

	/* Wait for holder to grab mutex */
	while (rt_srv.phase < 1) {
		usleep(1000);
	}

	priority(RT_P_SERVER);

	/* Signal readiness via pipe */
	char c = 'R';
	write(args->pipe_fd, &c, 1);
	close(args->pipe_fd);

	/* Server loop: receive IPC (gets SC donation at caller's prio),
	 * then try to lock mutex -> blocks until holder releases */
	for (;;) {
		int err = msgRecv(port, &msg, &rid);
		if (err < 0) {
			_exit(0);
		}

		rt_srv.phase = 2; /* signal: about to contend on mutex */

		mutexLock(rt_srv.mutex);
		mutexUnlock(rt_srv.mutex);

		msg.o.err = 0;
		msgRespond(port, &msg, rid);

		/* Advance round: helper threads prepare for next iteration */
		rt_srv.phase = 0;
		rt_srv.round++;

		/* Wait for holder to re-lock mutex */
		while (rt_srv.phase < 1) {
			usleep(1000);
		}
	}
}


static pid_t spawn_rt_server(const char *dev_path, oid_t *out_oid,
		int holder_prio, int medium_prio, int holder_work_us, int spawn_medium)
{
	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd) < 0) {
		return -1;
	}

	fflush(stdout);
	fflush(stderr);
	pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}

	if (pid == 0) {
		close(pipefd[0]);
		struct rt_server_args args = {
			.pipe_fd = pipefd[1],
			.dev_path = dev_path,
			.holder_prio = holder_prio,
			.medium_prio = medium_prio,
			.holder_work_us = holder_work_us,
			.spawn_medium = spawn_medium,
		};
		rt_server_process(&args);
		_exit(0);
	}

	/* Parent: wait for server to be ready */
	close(pipefd[1]);
	char c;
	read(pipefd[0], &c, 1);
	close(pipefd[0]);

	/* Lookup the server's port */
	if (wait_for_dev(dev_path, out_oid, 5000) < 0) {
		server_kill(pid);
		return -1;
	}

	return pid;
}


/*
 * Benchmark 1: Three-Task Priority Inversion
 *
 * Scenario:
 *   Client H (P1) --IPC--> Server S (base P4):
 *     S receives IPC, runs on H's scheduling context (effective P1)
 *     S tries to lock mutex held by L (base P6)
 *     Interferer M (P3) is CPU-bound
 *
 * Correct behavior (PI propagates SC priority through mutex):
 *   S(P1) blocks on mutex -> L boosted to P1 -> M(P3) cannot preempt L
 *   -> L releases quickly -> S responds -> H unblocks
 *   Expected latency: ~holder_work_us + IPC overhead
 *
 * Broken behavior (PI only sees thread base prio P4):
 *   S(base P4) blocks on mutex -> L boosted to P4 -> M(P3) preempts L
 *   -> H waits for M's entire busy loop (~200ms) = unbounded inversion
 *   Expected latency: ~200ms (RT_BUSY_ITERS loop time)
 *
 * We run this benchmark repeatedly and report latency distribution.
 * A correct implementation shows low, consistent latency.
 * A broken one shows p99/max >> baseline (typically 100-1000x).
 */
static int bench_rt_three_task(int iterations)
{
	oid_t oid;
	pid_t srv_pid;
	time_t *samples;
	char dev_path[64];

	make_path(dev_path, sizeof(dev_path), "rt3t");

	/* holder_work = 1ms (short critical section), medium-prio interferer active */
	srv_pid = spawn_rt_server(dev_path, &oid, RT_P_LOW, RT_P_MEDIUM, 1000, 1);
	if (srv_pid < 0) {
		fprintf(stderr, "  ERROR: spawn_rt_server failed\n");
		return -1;
	}

	samples = malloc(sizeof(time_t) * (size_t)iterations);
	if (samples == NULL) {
		server_kill(srv_pid);
		return -1;
	}

	priority(RT_P_HIGH);

	/* Warmup */
	for (int i = 0; i < WARMUP_ITERS; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		msgSend(oid.port, &msg);
	}

	for (int i = 0; i < iterations; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;

		time_t t0 = gettime_us();
		msgSend(oid.port, &msg);
		time_t t1 = gettime_us();

		samples[i] = t1 - t0;
	}

	priority(RT_P_LOW);

	report_latency("rt 3-task", samples, iterations);

	free(samples);
	server_kill(srv_pid);
	return 0;
}


/*
 * Benchmark 2: Three-Task Baseline (no interferer)
 *
 * Same as above but without the medium-prio CPU hog.
 * This gives a baseline: what the latency looks like when there's no
 * risk of inversion. Comparing this to Benchmark 1 directly shows
 * whether M causes any degradation.
 */
static int bench_rt_three_task_baseline(int iterations)
{
	oid_t oid;
	pid_t srv_pid;
	time_t *samples;
	char dev_path[64];

	make_path(dev_path, sizeof(dev_path), "rt3b");

	/* Same setup but no medium-prio interferer */
	srv_pid = spawn_rt_server(dev_path, &oid, RT_P_LOW, 0, 1000, 0);
	if (srv_pid < 0) {
		return -1;
	}

	samples = malloc(sizeof(time_t) * (size_t)iterations);
	if (samples == NULL) {
		server_kill(srv_pid);
		return -1;
	}

	priority(RT_P_HIGH);

	for (int i = 0; i < WARMUP_ITERS; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		msgSend(oid.port, &msg);
	}

	for (int i = 0; i < iterations; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;

		time_t t0 = gettime_us();
		msgSend(oid.port, &msg);
		time_t t1 = gettime_us();

		samples[i] = t1 - t0;
	}

	priority(RT_P_LOW);

	report_latency("rt 3-task baseline", samples, iterations);

	free(samples);
	server_kill(srv_pid);
	return 0;
}


/*
 * Benchmark 3: Cascaded IPC → Lock → IPC
 *
 * Scenario:
 *   Client H (P1) --IPC--> Server S1:
 *     S1 (base P4) receives IPC at effective P1, blocks on mutex
 *     Holder W (base P5) holds mutex, does IPC to backend S2 (base P7)
 *
 * Priority chain: H(P1) -SC-> S1(eff P1) -mutex PI-> W(should be P1) -SC-> S2
 *
 * Correct: W gets boosted to P1 via mutex PI, then W's outbound IPC
 * donates P1 to S2. Everything completes at P1, no inversion.
 *
 * Broken: W only gets boosted to S1's base prio (P4) or not at all.
 * A medium-prio CPU hog (P3) can preempt W, causing inversion for H.
 *
 * This is the hardest scenario: priority must propagate through
 * IPC SC donation → mutex PI → IPC SC donation (three hops).
 */


/* Cascaded server: receives IPC, then does outbound IPC to backend while
 * holding a mutex that the main server thread is blocked on. */
struct rt_casc_args {
	int pipe_fd;
	const char *dev_path;
	const char *backend_path;
	int spawn_medium;
};


static struct {
	handle_t mutex;
	volatile int phase;
	volatile int round;
	oid_t backend_oid;
	char stack[3][4096] __attribute__((aligned(8)));
} rt_casc;


static void rt_casc_worker(void *arg)
{
	int my_round = 0;

	for (;;) {
		/* Thread W (P5): holds mutex, waits for server to contend,
		 * then does IPC to backend while boosted */
		mutexLock(rt_casc.mutex);
		rt_casc.phase = 1; /* mutex held */

		/* Wait for server thread to receive IPC and try to lock */
		while (rt_casc.phase < 2) {
			usleep(1000);
		}
		usleep(20 * 1000); /* ensure server blocks on mutex */

		/* Do IPC to backend (should carry boosted priority) */
		msg_t fwd = { 0 };
		fwd.type = mtRead;
		msgSend(rt_casc.backend_oid.port, &fwd);

		mutexUnlock(rt_casc.mutex);

		/* Wait for next round */
		my_round++;
		while (rt_casc.round < my_round) {
			usleep(1000);
		}
	}
}


static void rt_casc_medium(void *arg)
{
	int my_round = 0;

	for (;;) {
		/* CPU hog at medium priority */
		while (rt_casc.phase < 2) {
			usleep(1000);
		}
		usleep(10 * 1000);

		for (volatile int i = 0; i < RT_BUSY_ITERS && rt_casc.phase != 0; i++) {
			;
		}

		/* Wait for next round */
		my_round++;
		while (rt_casc.round < my_round) {
			usleep(1000);
		}
	}
}


static void rt_casc_server_process(struct rt_casc_args *args)
{
	uint32_t port;
	oid_t oid;
	msg_t msg = { 0 };
	msg_rid_t rid;

	if (portCreate(&port) < 0) {
		_exit(3);
	}
	oid.port = port;
	oid.id = 0;
	if (create_dev(&oid, args->dev_path) < 0) {
		_exit(3);
	}

	/* Wait for backend server to be available */
	while (lookup(args->backend_path, NULL, &rt_casc.backend_oid) < 0) {
		usleep(10 * 1000);
	}

	mutexCreate(&rt_casc.mutex);
	rt_casc.phase = 0;
	rt_casc.round = 0;

	/* Spawn long-lived worker that holds the mutex */
	beginthread(rt_casc_worker, 5,
			rt_casc.stack[0], sizeof(rt_casc.stack[0]), NULL);

	if (args->spawn_medium != 0) {
		beginthread(rt_casc_medium, RT_P_MEDIUM,
				rt_casc.stack[1], sizeof(rt_casc.stack[1]), NULL);
	}

	/* Wait for worker to grab mutex */
	while (rt_casc.phase < 1) {
		usleep(1000);
	}

	priority(RT_P_SERVER);

	char c = 'R';
	write(args->pipe_fd, &c, 1);
	close(args->pipe_fd);

	for (;;) {
		int err = msgRecv(port, &msg, &rid);
		if (err < 0) {
			_exit(0);
		}

		rt_casc.phase = 2; /* about to contend */

		/* Block on mutex held by worker */
		mutexLock(rt_casc.mutex);
		mutexUnlock(rt_casc.mutex);

		msg.o.err = 0;
		msgRespond(port, &msg, rid);

		/* Advance round: worker prepares for next iteration */
		rt_casc.phase = 0;
		rt_casc.round++;

		while (rt_casc.phase < 1) {
			usleep(1000);
		}
	}
}


static int bench_rt_cascaded(int iterations, int with_interferer)
{
	oid_t oid, backend_oid;
	pid_t backend_pid, srv_pid;
	time_t *samples;
	char dev_path[64], backend_path[64];
	int pipefd[2];

	make_path(dev_path, sizeof(dev_path), "rtcas");
	make_path(backend_path, sizeof(backend_path), "rtcbe");

	/* Spawn backend echo server (simple, no lock) */
	backend_pid = server_spawn_echo(backend_path, 0, RT_P_LOWEST);
	if (backend_pid < 0) {
		return -1;
	}
	if (wait_for_dev(backend_path, &backend_oid, 5000) < 0) {
		server_kill(backend_pid);
		return -1;
	}

	/* Spawn cascaded server */
	if (pipe(pipefd) < 0) {
		server_kill(backend_pid);
		return -1;
	}

	fflush(stdout);
	fflush(stderr);
	srv_pid = fork();
	if (srv_pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		server_kill(backend_pid);
		return -1;
	}

	if (srv_pid == 0) {
		close(pipefd[0]);
		struct rt_casc_args args = {
			.pipe_fd = pipefd[1],
			.dev_path = dev_path,
			.backend_path = backend_path,
			.spawn_medium = with_interferer,
		};
		rt_casc_server_process(&args);
		_exit(0);
	}

	close(pipefd[1]);
	char c;
	read(pipefd[0], &c, 1);
	close(pipefd[0]);

	if (wait_for_dev(dev_path, &oid, 5000) < 0) {
		server_kill(srv_pid);
		server_kill(backend_pid);
		return -1;
	}

	samples = malloc(sizeof(time_t) * (size_t)iterations);
	if (samples == NULL) {
		server_kill(srv_pid);
		server_kill(backend_pid);
		return -1;
	}

	priority(RT_P_HIGH);

	for (int i = 0; i < WARMUP_ITERS; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		msgSend(oid.port, &msg);
	}

	for (int i = 0; i < iterations; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;

		time_t t0 = gettime_us();
		msgSend(oid.port, &msg);
		time_t t1 = gettime_us();

		samples[i] = t1 - t0;
	}

	priority(RT_P_LOW);

	const char *label = with_interferer ? "rt IPC->lock->IPC" : "rt IPC->lock->IPC (baseline)";
	report_latency(label, samples, iterations);

	free(samples);
	server_kill(srv_pid);
	server_kill(backend_pid);
	return 0;
}


/*
 * Benchmark 4: Multi-client contention with priority differentiation
 *
 * Scenario:
 *   Client H (P1) -- measures latency
 *   Client M1 (P3) -- concurrent load (different process)
 *   Client M2 (P3) -- concurrent load (different process)
 *   Server with mutex-holding pattern (same as three-task)
 *
 * All clients send IPC to the same server which blocks on a mutex.
 * The high-prio client should not see its latency degraded by the
 * medium-prio clients' traffic. If the server processes clients
 * in priority order (via SC donation), H always gets served first.
 *
 * We measure H's latency while M1/M2 continuously hammer the server.
 * A correct implementation keeps H's latency close to the baseline.
 * A broken one shows H queued behind M1/M2.
 */
static int bench_rt_multi_client(int iterations)
{
	oid_t oid;
	pid_t srv_pid, load_pids[2];
	time_t *samples;
	char dev_path[64];
	int i;

	make_path(dev_path, sizeof(dev_path), "rtmc");

	/* Server with short critical section, no dedicated interferer */
	srv_pid = spawn_rt_server(dev_path, &oid, RT_P_LOW, 0, 500, 0);
	if (srv_pid < 0) {
		return -1;
	}

	/* Spawn medium-prio load client processes */
	for (i = 0; i < 2; i++) {
		fflush(stdout);
		fflush(stderr);
		load_pids[i] = fork();
		if (load_pids[i] < 0) {
			/* cleanup */
			for (int j = 0; j < i; j++) {
				server_kill(load_pids[j]);
			}
			server_kill(srv_pid);
			return -1;
		}

		if (load_pids[i] == 0) {
			/* Medium-prio client: hammer the server continuously */
			priority(RT_P_MEDIUM);
			oid_t loid;
			if (wait_for_dev(dev_path, &loid, 5000) < 0) {
				_exit(1);
			}

			for (;;) {
				msg_t msg = { 0 };
				msg.type = mtRead;
				if (msgSend(loid.port, &msg) < 0) {
					_exit(0);
				}
			}
		}
	}

	/* Let load clients start hammering */
	usleep(50 * 1000);

	samples = malloc(sizeof(time_t) * (size_t)iterations);
	if (samples == NULL) {
		for (i = 0; i < 2; i++) {
			server_kill(load_pids[i]);
		}
		server_kill(srv_pid);
		return -1;
	}

	priority(RT_P_HIGH);

	/* Warmup */
	for (i = 0; i < WARMUP_ITERS; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;
		msgSend(oid.port, &msg);
	}

	for (i = 0; i < iterations; i++) {
		msg_t msg = { 0 };
		msg.type = mtRead;

		time_t t0 = gettime_us();
		msgSend(oid.port, &msg);
		time_t t1 = gettime_us();

		samples[i] = t1 - t0;
	}

	priority(RT_P_LOW);

	report_latency("rt multi-client", samples, iterations);

	free(samples);
	for (i = 0; i < 2; i++) {
		server_kill(load_pids[i]);
	}
	server_kill(srv_pid);
	return 0;
}


/*
 * Benchmark 5: Nested lock then IPC (intra-process)
 *
 * Scenario (single process, multiple threads + external server):
 *   Thread L (P6): locks mutex M, then does IPC to external echo server
 *   Thread H (P1): blocks on mutex M → mutex PI boosts L to P1
 *   Interferer M_thr (P3): CPU-bound busy loop
 *
 * When L does outbound IPC while boosted by mutex PI, the IPC SC
 * donation should carry the boosted prio (P1), not L's base prio (P6).
 * The echo server runs at P7; if it receives SC at P1, it runs above M_thr.
 *
 * Correct: L is boosted to P1, does IPC (donates P1 to server), server
 * runs above M_thr, responds quickly, L unlocks, H gets mutex.
 *
 * Broken: L's IPC only carries base prio P6, server runs below M_thr (P3),
 * H is delayed by M_thr's entire busy loop.
 */

static struct {
	handle_t mutex;
	volatile int phase;
	volatile int round;
	volatile int quit;
	oid_t backend_oid;
	time_t ipc_latency; /* measured IPC round-trip inside L while boosted */
	char stack[3][4096] __attribute__((aligned(8)));
} rt_nested;


static void rt_nested_low(void *arg)
{
	int my_round = 0;

	while (rt_nested.quit == 0) {
		/* Wait for next round */
		while (rt_nested.round <= my_round && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			break;
		}
		my_round++;

		/* Thread L (P6): lock mutex, wait for H to contend (which boosts us),
		 * then do IPC to the echo server */
		mutexLock(rt_nested.mutex);
		rt_nested.phase = 1; /* mutex held */

		/* Wait for H to block on our mutex */
		while (rt_nested.phase < 2 && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			mutexUnlock(rt_nested.mutex);
			break;
		}
		usleep(20 * 1000); /* let H actually block */

		/* We should now be boosted to P1. Do IPC. */
		msg_t msg = { 0 };
		msg.type = mtRead;

		time_t t0 = gettime_us();
		msgSend(rt_nested.backend_oid.port, &msg);
		time_t t1 = gettime_us();

		rt_nested.ipc_latency = t1 - t0;

		mutexUnlock(rt_nested.mutex);
	}
	endthread();
}


static void rt_nested_high(void *arg)
{
	int my_round = 0;

	while (rt_nested.quit == 0) {
		while (rt_nested.round <= my_round && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			break;
		}
		my_round++;

		/* Thread H (P1): wait for L to hold mutex, then block on it */
		while (rt_nested.phase < 1 && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			break;
		}
		rt_nested.phase = 2; /* about to contend */

		mutexLock(rt_nested.mutex);
		mutexUnlock(rt_nested.mutex);
		rt_nested.phase = 3; /* done */
	}
	endthread();
}


static void rt_nested_medium(void *arg)
{
	int my_round = 0;

	while (rt_nested.quit == 0) {
		while (rt_nested.round <= my_round && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			break;
		}
		my_round++;

		while (rt_nested.phase < 2 && rt_nested.quit == 0) {
			usleep(1000);
		}
		if (rt_nested.quit != 0) {
			break;
		}
		usleep(10 * 1000);

		/* CPU hog at P3 */
		for (volatile int i = 0; i < RT_BUSY_ITERS && rt_nested.phase == 2; i++) {
			;
		}
	}
	endthread();
}


static int bench_rt_nested_lock_ipc(int iterations, int with_interferer)
{
	pid_t backend_pid;
	time_t *samples;
	char backend_path[64];

	make_path(backend_path, sizeof(backend_path), "rtnbe");

	/* Spawn echo backend server at lowest prio */
	backend_pid = server_spawn_echo(backend_path, 0, RT_P_LOWEST);
	if (backend_pid < 0) {
		return -1;
	}
	if (wait_for_dev(backend_path, &rt_nested.backend_oid, 5000) < 0) {
		server_kill(backend_pid);
		return -1;
	}

	samples = malloc(sizeof(time_t) * (size_t)iterations);
	if (samples == NULL) {
		server_kill(backend_pid);
		return -1;
	}

	mutexCreate(&rt_nested.mutex);
	rt_nested.phase = 0;
	rt_nested.round = 0;
	rt_nested.quit = 0;
	rt_nested.ipc_latency = 0;

	/* Spawn long-lived threads (reused across all iterations) */
	beginthread(rt_nested_low, RT_P_LOW,
			rt_nested.stack[0], sizeof(rt_nested.stack[0]), NULL);

	beginthread(rt_nested_high, RT_P_HIGH,
			rt_nested.stack[1], sizeof(rt_nested.stack[1]), NULL);

	if (with_interferer != 0) {
		beginthread(rt_nested_medium, RT_P_MEDIUM,
				rt_nested.stack[2], sizeof(rt_nested.stack[2]), NULL);
	}

	for (int iter = 0; iter < iterations; iter++) {
		rt_nested.phase = 0;

		time_t t0 = gettime_us();
		rt_nested.round = iter + 1; /* start round */

		/* Wait for completion */
		while (rt_nested.phase < 3) {
			usleep(1000);
		}
		time_t t1 = gettime_us();

		samples[iter] = t1 - t0;
	}

	rt_nested.quit = 1;
	usleep(50 * 1000); /* let threads notice quit flag */

	resourceDestroy(rt_nested.mutex);

	const char *label = with_interferer ? "rt nested lock->IPC" : "rt nested lock->IPC (baseline)";
	report_latency(label, samples, iterations);

	free(samples);
	server_kill(backend_pid);
	return 0;
}


static void run_rt_benchmarks(void)
{
	priority(RT_P_LOW);

	fprintf(stderr, "\n=== Priority inversion benchmarks (IPC + mutex PI) ===\n");
	fprintf(stderr, "  (lower number = higher priority; P1 highest)\n\n");

	bench_rt_three_task_baseline(RT_ITERS);
	bench_rt_three_task(RT_ITERS);

	fprintf(stderr, "\n");

	bench_rt_cascaded(RT_ITERS / 2, 0);
	bench_rt_cascaded(RT_ITERS / 2, 1);

	fprintf(stderr, "\n");

	bench_rt_multi_client(RT_ITERS);

	fprintf(stderr, "\n");

	bench_rt_nested_lock_ipc(RT_ITERS / 2, 0);
	bench_rt_nested_lock_ipc(RT_ITERS / 2, 1);
}


/* ================================================================
 * Main entry point
 * ================================================================ */


int bench_run_all(int csv)
{
	size_t d, i;

	csv_output = csv;

	if (csv_output != 0) {
		printf("name,iterations,total_us,avg_us,min_us,med_us,p99_us,max_us,jitter_us\n");
	}

	fprintf(stderr, "\n=== Mandatory cost benchmarks ===\n");
	bench_mandatory_syscall_enter_exit(100000);

#if 1
	fprintf(stderr, "\n=== Raw IPC round-trip benchmarks ===\n");
	for (d = 0; d < sizeof(raw_depths) / sizeof(raw_depths[0]); d++) {
		for (i = 0; i < sizeof(raw_specs) / sizeof(raw_specs[0]); i++) {
			const struct raw_bench_spec *s = &raw_specs[i];
			bench_raw_ipc(raw_depths[d], s->payload, s->iterations, s->use_rr, s->direction);
		}
	}

	// return 0;

	fprintf(stderr, "\n=== POSIX-level IPC benchmarks ===\n");
	if (posix_setup() < 0) {
		fprintf(stderr, "  ERROR: failed to set up POSIX server\n");
		return -1;
	}

	bench_posix_open_close(1000);

	for (i = 0; i < sizeof(posix_rw_specs) / sizeof(posix_rw_specs[0]); i++) {
		const struct posix_rw_spec *s = &posix_rw_specs[i];
		bench_posix_read(s->size, s->iterations);
		bench_posix_write(s->size, s->iterations);
	}

	bench_posix_stat(1000);

	posix_teardown();

	fprintf(stderr, "\n=== Name resolution / port benchmarks ===\n");

	/* Set up a server for lookup benchmarks */
	if (posix_setup() < 0) {
		fprintf(stderr, "  ERROR: failed to set up server for lookup\n");
		return -1;
	}

	bench_lookup_cached(1000);
	bench_lookup_root(1000);
	bench_portcreate_destroy(1000);
	bench_portregister_unregister(200);

	posix_teardown();

#endif

	run_rt_benchmarks();

	fprintf(stderr, "\n=== Done ===\n");

	return 0;
}
