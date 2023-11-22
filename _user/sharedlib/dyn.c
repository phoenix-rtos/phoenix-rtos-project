/*
 * Phoenix-RTOS
 *
 * sharedlib
 *
 * Example of user application
 *
 * Copyright 2024 Phoenix Systems
 * Author: Hubert badocha
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