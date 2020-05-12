/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Disk routines
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _DISK_H_
#define _DISK_H_


extern s32 disk_open(u16 dn, char *name, u32 flags);


/* Function reads bytes from file specified by starting block number (handle) */
extern s32 disk_read(u16 dn, s32 handle, u32 *pos, u8 *buff, u32 len);


extern s32 disk_close(u16 dn, s32 handle);


#endif
