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

#include <arch.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/debug.h>


int do_msg(oid_t oid, void *data, size_t size)
{
	memset(data, 'a', size);
	msg_t msg = {
		.oid = oid,
		.type = mtWrite,
		.i = {
			.data = data,
			.size = size,
			.io.offs = 0,
		}
	};

	return msgSend(oid.port, &msg);
}

extern void sbi_putchar(void);

int main(void)
{
	void *buff = mmap(NULL, 450 * _PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (buff == MAP_FAILED) {
		debug("mmap failed\n");
		return -1;
	}
	oid_t oid = { 0 };

	sleep(1);

	for (size_t i = _PAGE_SIZE; i < 450 * _PAGE_SIZE; i += (_PAGE_SIZE / 2)) {
		if (do_msg(oid, buff, i) < 0) {
			debug("do_msg failed\n");
			return -1;
		}
	}

	sbi_putchar();


	return 0;
}
