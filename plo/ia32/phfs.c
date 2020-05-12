/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * PHoenix FileSystem
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2005 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "config.h"
#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "phfs.h"
#include "phoenixd.h"
#include "disk.h"


struct {
	s32 (*open)(u16, char *, u32);
	s32 (*read)(u16, s32, u32 *, u8 *, u32);
	s32 (*close)(u16, s16);
	unsigned int dn;
} phfs_handlers[PDN_HD3 + 1];


s32 phfs_open(u16 pdn, char *name, u32 flags)
{
	if (pdn > PDN_HD3)
		return ERR_ARG;

	return phfs_handlers[pdn].open(phfs_handlers[pdn].dn, name, flags);
}


s32 phfs_read(u16 pdn, s32 handle, u32 *pos, u8 *buff, u32 len)
{
	if (pdn > PDN_HD3)
		return ERR_ARG;

	return phfs_handlers[pdn].read(phfs_handlers[pdn].dn, handle, pos, buff, len);
}


s32 phfs_close(u16 pdn, s32 handle)
{
	if (pdn > PDN_HD3)
		return ERR_ARG;

	return phfs_handlers[pdn].close(phfs_handlers[pdn].dn, handle);
}


void phfs_init(void)
{
	unsigned int i;

	/* Handlers for com devices */
	for (i = 0; i < 2; i++) {
		phfs_handlers[PDN_COM1 + i].open = phoenixd_open;
		phfs_handlers[PDN_COM1 + i].read = phoenixd_read;
		phfs_handlers[PDN_COM1 + i].close = phoenixd_close;
		phfs_handlers[PDN_COM1 + i].dn = i;
	}

	/* Handlers for floppy disks */
	phfs_handlers[PDN_FLOPPY].open = disk_open;
	phfs_handlers[PDN_FLOPPY].read = disk_read;
	phfs_handlers[PDN_FLOPPY].close = disk_close;
	phfs_handlers[PDN_FLOPPY].dn = 0;

	/* Handlers for hard disks */
	for (i = 0; i < 4; i++) {
		phfs_handlers[PDN_HD0 + i].open = disk_open;
		phfs_handlers[PDN_HD0 + i].read = disk_read;
		phfs_handlers[PDN_HD0 + i].close = disk_close;
		phfs_handlers[PDN_HD0 + i].dn = 0x80 + i;
	}

	return;
}
