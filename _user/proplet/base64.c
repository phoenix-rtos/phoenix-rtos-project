/*
 * Phoenix-RTOS
 *
 * base64 encoder and decoder - implementation
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <errno.h>

#include "base64.h"

/* clang-format off */
unsigned char b64table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};


unsigned char b64rev[] = {
	['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
	['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
	['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
	['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
	['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
	['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
	['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
	['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
	['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
	['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
	['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63,
};
/* clang-format on */


int base64_encode(const unsigned char *in, size_t insz, unsigned char *out, size_t outsz)
{
	unsigned char cur = 0, leftovers = 0;
	size_t outcur = 0;

	if (in == NULL || insz == 0 || out == NULL || outsz == 0) {
		return -EINVAL;
	}

	for (int i = 0; i < insz;) {
		switch (leftovers) {
			case 0:
				cur = (in[i] & 0xfc) >> 2;
				leftovers = 2;
				break;
			case 2:
				cur = (in[i] & 0x3) << 4;
				if (i + 1 < insz) {
					cur |= (in[++i] & 0xf0) >> 4;
				}
				leftovers = 4;
				break;
			case 4:
				cur = (in[i] & 0xf) << 2;
				if (i + 1 < insz) {
					cur |= (in[++i] & 0xc0) >> 6;
					if (outcur < outsz) {
						out[outcur] = b64table[cur];
					}
					outcur++;
					cur = (in[++i] & 0xfc) >> 2;
				}
				leftovers = 0;
				break;
		}
		if (outcur < outsz) {
			out[outcur] = b64table[cur];
		}
		outcur++;
		if (i + 1 >= insz) {
			i++;
		}
	}

	switch (leftovers) {
		case 2:
			out[outcur++] = '=';
			break;
		case 4:
			out[outcur++] = '=';
			out[outcur++] = '=';
	}

	return 0;
}


int base64_decode(const unsigned char *in, size_t insz, unsigned char *out, size_t outsz)
{
	char cur = 0;
	int leftovers = 0;
	size_t outcur = 0;

	if (in == NULL || insz == 0) {
		return -EINVAL;
	}

	for (int i = 0; i < insz;) {
		if (in[i] == '=') {
			break;
		}

		switch (leftovers) {
			case 0:
				cur = (b64rev[in[i]]) << 2;
				if (i + 1 >= insz) {
					return -EINVAL;
				}
				cur |= (b64rev[in[++i]] & 0x30) >> 4;
				leftovers = 4;
				break;
			case 4:
				cur = (b64rev[in[i]] & 0x0f) << 4;
				if (i + 1 >= insz) {
					return -EINVAL;
				}
				if (in[i + 1] == '=' && in[i + 2] == '=') {
					i += 2;
					continue;
				}
				cur |= (b64rev[in[++i]] & 0x3c) >> 2;
				leftovers = 2;
				break;
			case 2:
				cur = (b64rev[in[i]] & 0x03) << 6;
				if (i + 1 >= insz) {
					return -EINVAL;
				}
				if (in[i + 1] == '=') {
					i++;
					continue;
				}
				cur |= (b64rev[in[++i]]);
				i++;
				leftovers = 0;
				break;
		}

		if (outcur < outsz && out != NULL) {
			out[outcur] = cur;
		}

		outcur++;

		if (i + 1 >= insz && in[i + 1] != '=') {
			i++;
		}
	}

	return outcur;
}
