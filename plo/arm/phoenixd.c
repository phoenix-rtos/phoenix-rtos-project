/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * phoenixd communication
 *
 * Copyright 2005 Pawel Pisarczyk
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
#include "serial.h"
#include "msg.h"
#include "phoenixd.h"


s32 phoenixd_open(u16 dn, char *name, u32 flags)
{
	msg_t smsg, rmsg;
	u16 l;
	u32 h;
	
	l = plostd_strlen(name) + 1;

	*(u32 *)smsg.data = flags;
	low_memcpy(&smsg.data[sizeof(u32)], name, l);
	l += sizeof(u32);

	msg_settype(&smsg, MSG_OPEN);
	msg_setlen(&smsg, l);

	if (msg_send(dn, &smsg, &rmsg) < 0)
		return ERR_PHFS_IO;
	if (msg_gettype(&rmsg) != MSG_OPEN)
		return ERR_PHFS_PROTO;
	if (msg_getlen(&rmsg) != sizeof(u32))
		return ERR_PHFS_PROTO;

	if (!(h = *(u32 *)rmsg.data))
		return ERR_PHFS_FILE;
	return h;
}


s32 phoenixd_read(u16 dn, s32 handle, u32 *pos, u8 *buff, u32 len)
{
	msg_t smsg, rmsg;
	msg_phoenixd_t *io;
	u16 hdrsz;
	u32 l;
	int k;
	
	/* Read from serial */
	io = (msg_phoenixd_t *)smsg.data;
	hdrsz = (u16)((u32)io->data - (u32)io);

	if ((handle <= 0) || (len > MSG_MAXLEN - hdrsz))
		return ERR_ARG;

	io->handle = handle;
	io->pos = *pos;
	io->len = len;

	msg_settype(&smsg, MSG_READ);
	msg_setlen(&smsg, hdrsz);
	
	if (msg_send(dn, &smsg, &rmsg) < 0)
		return ERR_PHFS_IO;
	
	if (msg_gettype(&rmsg) != MSG_READ) {
		return ERR_PHFS_PROTO;
	}
	io = (msg_phoenixd_t *)rmsg.data;  

	if ((long)io->len < 0) {
		return ERR_PHFS_FILE;
	}

	*pos = io->pos;
	l = min(io->len, msg_getlen(&rmsg) - hdrsz);
	low_memcpy(buff, io->data, l);

	return l;
}


s32 phoenixd_close(u16 dn, s32 handle)
{
	return ERR_NONE;
}
