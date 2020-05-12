/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * UART '550 driver
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
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

#ifndef _SERIAL_H_
#define _SERIAL_H_


/* Register bits */
#define IMR_THRE      0x02
#define IMR_DR        0x01

#define IIR_IRQPEND   0x01
#define IIR_THRE      0x02
#define IIR_DR        0x04

#define LCR_DLAB      0x80
#define LCR_D8N1      0x03
#define LCR_D8N2      0x07

#define MCR_OUT2      0x08

#define LSR_DR        0x01
#define LSR_THRE      0x20

#define COM_PINSEL	0xe002c000
#define COM_PINVAL	0x55
#define COM1_BASE   0xe000c000
#define COM1_IRQ    2
#define COM1_SRC    6
#define COM1_PMASK	0x
#define COM2_BASE   0xe0010000
#define COM2_IRQ    3
#define COM2_SRC    7

#define BPS_9600	97
#define BPS_19200   48
#define BPS_28800   32
#define BPS_38400   24
#define BPS_57600   16
#define BPS_115200  8


/* Driver data sizes */
#define NPORTS    2

#define RBUFFSZ   256
#define SBUFFSZ   256


extern int serial_read(unsigned int pn, u8 *buff, u16 len, u16 timeout);
extern int serial_write(unsigned int pn, u8 *buff, u16 len);
extern void serial_init(u16 speed);
extern void serial_done(void);

#endif
