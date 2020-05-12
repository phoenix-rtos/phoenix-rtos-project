/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Messaging routines
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _MSG_H_
#define _MSG_H_


/* Special characters */
#define MSG_MARK      0x7e
#define MSG_ESC       0x7d
#define MSG_ESCMARK   0x5e
#define MSG_ESCESC    0x5d


/* Transmission parameters */
#define MSGRECV_TIMEOUT  500     /* milliseconds */
#define MSGRECV_MAXRETR  3


#define MSG_HDRSZ   2 * 4
#define MSG_MAXLEN  512


typedef struct _msg_t {
	u32 csum;
	u32 type;
	u8  data[MSG_MAXLEN];
} msg_t;


/* Message types */
#define MSG_ERR    0


#define msg_settype(m, t)  ((m)->type = ((m)->type & ~0xffff) | (t & 0xffff))
#define msg_gettype(m)     ((u16)((m)->type & 0xffff))
#define msg_setlen(m, l)   ((m)->type = ((m)->type & 0xffff) | ((u32)l << 16))
#define msg_getlen(m)      ((u16)((m)->type >> 16))


extern int msg_send(u16 pn, msg_t *smsg, msg_t *rmsg);


#endif
