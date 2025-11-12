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

int add(int a, int b) {
	int c = a + b;
	printf("Add: %d\n", c);
	
	return c;
}


int main(void)
{
	printf("Hello World!!\n");

	add(1, 2);

	return 0;
}
