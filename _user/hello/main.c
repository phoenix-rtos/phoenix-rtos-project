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


void work(void)
{
	int numbers[1024];
	printf("I'm working...\n");
	for (int i = 0; i < 1024; i++) {
		numbers[i] = i;
	}

	while (1) {
		int r = rand() % 1024;
		for (int i = 0; i < 1024; i++) {
			numbers[i] += numbers[r];
		}
	}

	for (int i = 0; i < 1024; i++) {
		printf("%d ", numbers[i]);
	}
}


int main(void)
{
	// int* x;

	work();

	// printf("Hello World!!\n");
	// printf("My stack is at %p\n", &x);
	// printf("My main is at %p\n", &main);

	// sleep(1);

	// x = (int*)0x01009fcc;
	// printf("Changed stack value\n");

	// for(int i = 0; i < 1000; i++) {
	// 	int* z = malloc(1024);
	// 	if (z == NULL) {
	// 		printf("malloc failed\n");
	// 		return -1;
	// 	}
	// 	*z = i;
	// 	printf("Got addr %p, %d kB\n", z, i);
	// 	// free(z);

	// }


	// sleep(1);
	// printf("value of %p is %x\n", x, *x);

	return 0;
}
