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

#ifndef _PHOENIXD_H_
#define _PHOENIXD_H_

#include "msg.h"

/* Message types */
#define MSG_OPEN    1
#define MSG_READ    2
#define MSG_WRITE   3
#define MSG_COPY    4


typedef struct _msg_phoenixd_t {
  u32 handle;
  u32 pos;
  u32 len;
  u8  data[MSG_MAXLEN - 3 * sizeof(u32)];
} msg_phoenixd_t;


extern s32 phoenixd_open(u16 dn, char *name, u32 flags);


extern s32 phoenixd_read(u16 dn, s32 handle, u32 *pos, u8 *buff, u32 len);


extern s32 phoenixd_close(u16 dn, s32 handle);


#endif
