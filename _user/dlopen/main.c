/*
 * Phoenix-RTOS
 *
 * dlopen
 *
 * Example of user application using dlopen.
 *
 * Copyright 2024 Phoenix Systems
 * Author: Hubert Badocha
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stddef.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/debug.h>

int main(void)
{
	void *handle = dlopen("/usr/lib/libhello.so", RTLD_LAZY);
	if (handle == NULL) {
		printf("%s\n", dlerror());
		return 1;
	}
	int (*hello)(void) = (int (*)(void))dlsym(handle, "hello");
	if (hello == NULL) {
		printf("%s\n", dlerror());
		(void)dlclose(handle);
		return 1;
	}

	hello();

	(void)dlclose(handle);

	return 0;
}
