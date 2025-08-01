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
#include <stdint.h>

#define CBC 1
#include <tinyaes/aes.h>


void disp_buffor(uint8_t *buff, int size)
{
	for (int i = 0; i < size; i++) {
		printf("%02x ", buff[i]);
	}
	printf("\n");
}

int main(void)
{

	uint8_t key[16] = { 0x01 };
	struct AES_ctx ctx;
	(void)ctx;
	printf("Key: ");
	disp_buffor(key, 16);

	AES_init_ctx(&ctx, key);

	// uint8_t buffor1[32] = {0x2};
	// AES_ECB_encrypt(&ctx, buffor1);
	// AES_ECB_decrypt(&ctx, buffor1);
	// printf("ECB:\n");
	// disp_buffor(buffor1, 16);

	uint8_t iv[16] = { 0x69 };
	uint8_t buffor2[32] = { 0x02 };
	AES_ctx_set_iv(&ctx, iv);
	AES_CBC_encrypt_buffer(&ctx, buffor2, 32);
	// AES_CBC_decrypt_buffer(&ctx, buffor2, 32);
	printf("CBC:\n");
	disp_buffor(buffor2, 16);
	disp_buffor(buffor2 + 16, 16);

	return 0;
}
