/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * PHoenix FileSystem
 *
 * Copyright 2005 Pawel Pisarczyk
 * Copyright 2006 Radoslaw F. Wawrzusiak
 *
 * This file is part of Phoenix-RTOS.
 *
 * Phoenix-RTOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Phoenix-RTOS kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phoenix-RTOS kernel; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "phfs.h"
#include "phoenixd.h"


struct {
	s32 (*open)(u16, char *, u32);
	s32 (*read)(u16, s32, u32 *, u8 *, u32);
	s32 (*close)(u16, s32);
	unsigned int dn;
} phfs_handlers[PDN_COM2 + 1];


s32 phfs_open(u16 pdn, char *name, u32 flags)
{
	if (pdn > PDN_COM2)
		return ERR_ARG;

	return phfs_handlers[pdn].open(phfs_handlers[pdn].dn, name, flags);
}


s32 phfs_read(u16 pdn, s32 handle, u32 *pos, u8 *buff, u32 len)
{
	if (pdn > PDN_COM2)
		return ERR_ARG;

	return phfs_handlers[pdn].read(phfs_handlers[pdn].dn, handle, pos, buff, len);
}


s32 phfs_close(u16 pdn, s32 handle)
{
	if (pdn > PDN_COM2)
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

	return;
}
