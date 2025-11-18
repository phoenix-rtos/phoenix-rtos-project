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
 * 
 * %LICENSE%
 */

int add(int a, int b) {
	int c = a + b;
	
	return c;
}


int main(void)
{


	add(1, 2);

	return 0;
}
