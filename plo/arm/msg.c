/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Messaging routines
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
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

#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "serial.h"
#include "msg.h"


#define MSGREAD_DESYN   0
#define MSGREAD_FRAME   1


u32 msg_csum(msg_t *msg)
{
	unsigned int k;
	u32 csum;

	csum = 0;
	for (k = 0; k < MSG_HDRSZ + msg_getlen(msg); k++) {
		if (k >= sizeof(msg->csum))
			csum += (u32)(*((u8 *)msg + k));
	}
	return csum;
}


int msg_safewrite(u16 pn, u8 *buff, u16 len)
{
	int l;

	for (l = 0; len;) {
		if ((l = serial_write(pn, buff, len)) < 0)
			return ERR_MSG_IO;
		buff += l;
		len -= l;
	}
	return 0;
}


int msg_write(u16 pn, msg_t *msg)
{
	u8 *p = (u8 *)msg;
	char cs[2];
	u16 k;
	int res;

	/* Frame start */
	cs[0] = MSG_MARK;
	if ((res = msg_safewrite(pn, cs, 1)) < 0)
		return res;

	for (k = 0; k < MSG_HDRSZ + msg_getlen(msg); k++) {
		if ((p[k] == MSG_MARK) || (p[k] == MSG_ESC)) {
			cs[0] = MSG_ESC;
			if (p[k] == MSG_MARK)
				cs[1] = MSG_ESCMARK;
			else
				cs[1] = MSG_ESCESC;
			if ((res = msg_safewrite(pn, cs, 2)) < 0)
				return res;
		}
		else {
			if ((res = msg_safewrite(pn, &p[k], 1)) < 0)
				return res;
		}
	}
	return k;
}


int msg_read(u16 pn, msg_t *msg, u16 timeout, int *state)
{
	int escfl = 0;
	unsigned int l = 0;
	u8 c;
	
	for (;;) {
		if (serial_read(pn, &c, 1, timeout) < 0) {
			*state = MSGREAD_DESYN;
			return ERR_MSG_IO;
		}

		if (*state == MSGREAD_FRAME) {
			
			/* Return error if frame is to long */
			if (l == MSG_HDRSZ + MSG_MAXLEN) {
				*state = MSGREAD_DESYN;
				return ERR_MSG_IO;
			}

			/* Return error if terminator discovered */
			if (c == MSG_MARK)
				return ERR_MSG_IO;
			
			if (!escfl && (c == MSG_ESC)) {
				escfl = 1;
				continue;
			}
			if (escfl) {
				if (c == MSG_ESCMARK)
					c = MSG_MARK;
				if (c == MSG_ESCESC)
					c = MSG_ESC;
				escfl = 0;
			}
			*((u8 *)msg + l++) = c;

			/* Frame received */
			if ((l >= MSG_HDRSZ) && (l == msg_getlen(msg) + MSG_HDRSZ)) {
				*state = MSGREAD_DESYN;
				break;
			}
		}
		else {
			/* Synchronize */
			if (c == MSG_MARK)
				*state = MSGREAD_FRAME;
		}
	}

	/* Verify received message */
	if (msg->csum != msg_csum(msg))
		return ERR_MSG_IO;

	return l;
}


int msg_send(u16 pn, msg_t *smsg, msg_t *rmsg)
{
	unsigned int retr;
	int state = MSGREAD_DESYN;
	
	smsg->csum = msg_csum(smsg);
	for (retr = 0; retr < MSGRECV_MAXRETR; retr++) {
		if (msg_write(pn, smsg) < 0)
			continue;

		if ((msg_read(pn, rmsg, MSGRECV_TIMEOUT, &state)) > 0) {
			return 0;
		}
	}

	return ERR_MSG_IO;
}

