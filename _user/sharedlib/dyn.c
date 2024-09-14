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

#include "dyn.h"


extern void *_DYNAMIC;


void *dyn(void)
{
	return &_DYNAMIC;
}
