/*
 * Phoenix-RTOS
 *
 * Hello World
 *
 * Example of user application
 *
 * Copyright 2021 Phoenix Systems
 * Author: Hubert Buczynski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <posix/utils.h>
#include "audio.h"


static int server_handleOpen(oid_t *oid)
{
	(void)oid;
	return 0;
}


static int server_handleClose(oid_t *oid)
{
	(void)oid;
	return 0;
}


static ssize_t server_handleWrite(oid_t *oid, const void *data, size_t len, off_t offset)
{
	(void)offset;
	if (len == 1) {
		fprintf(stderr, "No input data provided\n");
		return (ssize_t)len;
	}
	else if (len != 2) {
		fprintf(stderr, "More than 1 latter provided\n");
		return (ssize_t)len;
	}

	char latter = ((char *)data)[0];

	const uint16_t low_freq = 170;
	const uint8_t low_freq_octave = 4;
	const uint16_t high_freq = 1000;
	const uint8_t high_freq_octave = 7;

	if (latter >= 'a' && latter <= 'j') {
		adlib_playVoice(0, low_freq, low_freq_octave);
		sleep((latter - 'a') + 1);
		adlib_stopVoice(0);
	}
	else if (latter >= 'A' && latter <= 'J') {
		adlib_playVoice(0, high_freq, high_freq_octave);
		sleep((latter - 'A') + 1);
		adlib_stopVoice(0);
	}
	else {
		fprintf(stderr, "Pass latter from range a-j or A-J\n");
	}

	return (ssize_t)len;
}


__attribute__((noreturn)) static void server_msgLoop(oid_t *oid)
{
	msg_t msg;
	msg_rid_t rid;

	for (;;) {
		int err = msgRecv(oid->port, &msg, &rid);
		if (err < 0) {
			if (err == -EINTR) {
				continue;
			}
			else {
				fprintf(stderr, "bepper_server: msgRecv returned %d (%s)\n", err, strerror(err));
				exit(EXIT_FAILURE);
			}
		}

		switch (msg.type) {
			case mtOpen:
				msg.o.err = server_handleOpen(&msg.oid);
				break;

			case mtClose:
				msg.o.err = server_handleClose(&msg.oid);
				break;

			case mtWrite:
				msg.o.err = server_handleWrite(&msg.oid, msg.i.data, msg.i.size, msg.i.io.offs);
				break;

			default:
				msg.o.err = -ENOSYS;
				break;
		}
		msgRespond(oid->port, &msg, rid);
	}
}


int main(void)
{
	oid_t oid;
	oid.id = 0;

	if (portCreate(&oid.port) < 0) {
		fprintf(stderr, "bepper_server: portCreate failed\n");
		return EXIT_FAILURE;
	}

	if (create_dev(&oid, "bepper") < 0) {
		fprintf(stderr, "bepper_server: create_dev failed\n");
		return EXIT_FAILURE;
	}

	if (adlib_init() != 0) {
		printf("Adlib init failed\n");
		return 1;
	}
	adlib_initVoice(0, 1);

	server_msgLoop(&oid);

	/* Never reached */
	if (adlib_init() != 0) {
		printf("Adlib deinit failed\n");
		return 1;
	}

	return EXIT_FAILURE;
}
