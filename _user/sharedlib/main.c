/*
 * Phoenix-RTOS
 *
 * sharedlib
 *
 * Example of user application using shared libraries.
 *
 * Copyright 2024 Phoenix Systems
 * Author: Hubert Badocha
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stdio.h>

#include "dyn.h"

int main(void)
{
	printf("Dynamic section pointer %p\n", dyn());

	return 0;
}
