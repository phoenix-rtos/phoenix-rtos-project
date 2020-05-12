/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * phoenixd communication
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2005 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
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
