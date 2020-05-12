/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * SHA-1 hash function (adopted from Libgcrypt sha1.c)
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2004, 2006 Pawel Pisarczyk
 * Author: Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _MAIN_SHA1_H_
#define _MAIN_SHA1_H_

extern int sha1verify(u8 *sign, u32 dataStart, u32 dataSize, int pdn, s32 h);

#endif
