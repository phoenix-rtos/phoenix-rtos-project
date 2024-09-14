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

#include "hello.h"

#include <stdio.h>


int hello(void)
{
	return printf("Hello shared world!\n");
}
