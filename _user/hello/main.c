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

int main(void)
{
	void *ptr = (void *)0x1;

	printf("ret = %d\n", sscanf("(nil)", "%p", &ptr));
	printf("ptr = %p\n", ptr);
	/* printf("ret = %d\n", sscanf("(nil) ", "%p", &ptr));
	printf("ptr = %p\n", ptr);
	printf("ret = %d\n", sscanf("(nil) 42", "%p", &ptr));
	printf("ptr = %p\n", ptr); */
}


	/* void *ptr = (void *)0x1;
	int xd = 0x1;
	printf("ptr = %p, %d\n", ptr, xd);
	printf("%d\n", sscanf("(nil) 42", "%p", &ptr));
	printf("ptr = %p, %d, %d\n", ptr, xd, errno);
	printf("%d\n", sscanf("(nil) ", "%p", &ptr));
	printf("ptr = %p, %d, %d\n", ptr, xd, errno);
	printf("WORKED BEFORE%d\n", sscanf("(nil)", "%p", &ptr));
	printf("ptr = %p, %d, %d\n", ptr, xd, errno);
	printf("%d\n", sscanf("(nil) 42", "%p %d", &ptr, &xd));
	printf("ptr = %p, %d, %d\n", ptr, xd, errno);
	sscanf("0 42", "%p %d", &ptr, &xd);
	printf("ptr = %p, %d\n", ptr, xd); */