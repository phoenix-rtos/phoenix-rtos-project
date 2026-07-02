/*
 * Phoenix-RTOS
 *
 * IPC benchmark - server implementations
 *
 * Provides echo, forwarding, and POSIX-style servers for benchmarking.
 *
 * Copyright 2026 Phoenix Systems
 * Author: Adam Greloch
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/threads.h>
#include <sys/wait.h>
#include <posix/utils.h>

#include "server.h"


#define POSIX_FILE_BUFSZ (64 * 1024)


static int setup_port(const char *dev_path, uint32_t *out_port)
{
	uint32_t port;
	oid_t oid;

	if (portCreate(&port) < 0) {
		return -1;
	}

	oid.port = port;
	oid.id = 0;

	if (create_dev(&oid, dev_path) < 0) {
		return -1;
	}

	*out_port = port;
	return 0;
}


/* ---- echo server ---- */


static void echo_loop(uint32_t port, int use_rr)
{
	msg_t msg = { 0 };
	msg_rid_t rid;

	for (;;) {
		int err;

#ifndef IPCBENCH_NO_RR
		if (use_rr != 0) {
			err = msgRespondAndRecv(port, &msg, &rid);
		}
		else
#endif
		{
			(void)use_rr;
			err = msgRecv(port, &msg, &rid);
		}

		if (err < 0) {
			exit(0); /* FIXME? */
		}

		/* Minimal work: just acknowledge the message */
		msg.o.err = 0;

#ifndef IPCBENCH_NO_RR
		if (use_rr == 0)
#endif
		{
			if (msgRespond(port, &msg, rid) < 0) {
				exit(2);
			}
		}
	}
}


pid_t server_spawn_echo(const char *dev_path, int use_respond_and_recv, int prio)
{
	pid_t pid;

	fflush(stdout);
	fflush(stderr);

	pid = fork();
	if (pid < 0) {
		return -1;
	}

	if (pid == 0) {
		uint32_t port;
		if (setup_port(dev_path, &port) < 0) {
			_exit(3);
		}
		priority(prio);
#ifndef IPCBENCH_NO_EXT_BUF
		if (msgSetup(2 * _PAGE_SIZE) < 0) {
			_exit(4);
		}
#endif
		echo_loop(port, use_respond_and_recv);
		_exit(0);
	}

	return pid;
}


/* ---- forwarding server ---- */


pid_t server_spawn_forwarder(const char *dev_path, uint32_t backend_port)
{
	pid_t pid;

	fflush(stdout);
	fflush(stderr);

	pid = fork();
	if (pid < 0) {
		return -1;
	}

	if (pid == 0) {
		uint32_t port;
		msg_t msg = { 0 };
		msg_rid_t rid;

		if (setup_port(dev_path, &port) < 0) {
			_exit(3);
		}

#ifndef IPCBENCH_NO_EXT_BUF
		/* TODO: make this a benchmark parameter */
		if (msgSetup(2 * _PAGE_SIZE) < 0) {
			_exit(4);
		}
#endif

		for (;;) {
			if (msgRecv(port, &msg, &rid) < 0) {
				_exit(0);
			}

			if (msgSend(backend_port, &msg) != 0) {
				_exit(4);
			}

			if (msgRespond(port, &msg, rid) < 0) {
				_exit(5);
			}
		}
	}

	return pid;
}


/* ---- POSIX-style server ---- */


static void posix_loop(uint32_t port)
{
	msg_t msg;
	msg_rid_t rid;

	/* Simple in-memory file buffer */
	char *filebuf = calloc(1, POSIX_FILE_BUFSZ);
	if (filebuf == NULL) {
		_exit(3);
	}

	size_t file_size = 0;

	for (;;) {
		memset(&msg, 0, sizeof(msg));

		int err = msgRecv(port, &msg, &rid);
		if (err < 0) {
			free(filebuf);
			_exit(0);
		}

		switch (msg.type) {
			case mtOpen:
				msg.o.err = 0;
				break;

			case mtClose:
				msg.o.err = 0;
				break;

			case mtRead: {
				off_t offs = msg.i.io.offs;
				size_t len = msg.o.size;

				if (offs < 0 || (size_t)offs >= file_size) {
					msg.o.err = 0; /* EOF */
					break;
				}

				if ((size_t)offs + len > file_size) {
					len = file_size - (size_t)offs;
				}

				if (msg.o.data != NULL && len > 0) {
					memcpy(msg.o.data, filebuf + offs, len);
				}

				msg.o.err = (int)len;
				break;
			}

			case mtWrite: {
				off_t offs = msg.i.io.offs;
				size_t len = msg.i.size;

				if (offs < 0) {
					msg.o.err = -EINVAL;
					break;
				}

				if ((size_t)offs + len > POSIX_FILE_BUFSZ) {
					len = POSIX_FILE_BUFSZ - (size_t)offs;
				}

				if (msg.i.data != NULL && len > 0) {
					memcpy(filebuf + offs, msg.i.data, len);
				}

				if ((size_t)offs + len > file_size) {
					file_size = (size_t)offs + len;
				}

				msg.o.err = (int)len;
				break;
			}

			case mtTruncate:
				file_size = 0;
				msg.o.err = 0;
				break;

			case mtGetAttr:
				/* Return file size for atSize (type == 3) */
				msg.o.attr.val = (long long)file_size;
				msg.o.err = 0;
				break;

			case mtGetAttrAll:
				memset(msg.o.data, 0, msg.o.size);
				msg.o.err = 0;
				break;

			case mtCreate:
				msg.o.create.oid.port = port;
				msg.o.create.oid.id = 0;
				msg.o.err = 0;
				break;

			case mtLookup:
				msg.o.lookup.fil.port = port;
				msg.o.lookup.fil.id = 0;
				msg.o.lookup.dev.port = port;
				msg.o.lookup.dev.id = 0;
				msg.o.err = (int)msg.i.size; /* consumed all chars */
				break;

			case mtLink:
			case mtUnlink:
				msg.o.err = 0;
				break;

			default:
				msg.o.err = -ENOSYS;
				break;
		}

		msgRespond(port, &msg, rid);
	}
}


pid_t server_spawn_posix(const char *dev_path)
{
	pid_t pid;

	fflush(stdout);
	fflush(stderr);

	pid = fork();
	if (pid < 0) {
		return -1;
	}

	if (pid == 0) {
		uint32_t port;
		if (setup_port(dev_path, &port) < 0) {
			_exit(3);
		}
		posix_loop(port);
		_exit(0);
	}

	return pid;
}


void server_kill(pid_t pid)
{
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}
