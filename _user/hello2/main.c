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
#include <stdlib.h>
#include <unistd.h>


void busyMem(void)
{
	int **x = malloc(1024 * sizeof(int *));
	printf("I'm busying memory...\n");
	while (1) {
		int i;
		for (i = 0; i < 1024; i++) {
			x[i] = malloc(1024);
			if (x[i] == NULL) {
				break;
			}
		}
		for (int j = 0; j < i; j++) {
			*(x[j]) = i * i + *x[j - 1];
		}
		// for (i = i-1; i >= 0; i--) {
		for (int j = 0; j < i; j++) {
			free(x[j]);
		}
	}
}


int main(void)
{
	busyMem();
	volatile int *x;
	printf("2Hello World!!\n");
	printf("2My stack is at %p\n", &x);
	printf("2My main is at %p\n", &main);

	x = (int *)0x4A4A4A4A;
	for (;;)
		sleep(1);

	// x = (int*)0x00469fcc;
	// printf("Changed stack value\n");


	// sleep(1);
	// printf("value of %p is %x\n", x, *x);


	return 0;
}
