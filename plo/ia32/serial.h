/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * UART 16450 driver
 *
 * Copyright 2012, 2020 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_


/* UART registers */
#define REG_RBR     0
#define REG_THR     0
#define REG_IMR     1
#define REG_IIR     2
#define REG_LCR     3
#define REG_MCR     4
#define REG_LSR     5
#define REG_MSR     6
#define REG_ADR     7
#define REG_LSB     0
#define REG_MSB     1
#define REG_FCR     2


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

#define FCR_ENABLE    0x01
#define FCR_RCVRCLR   0x02
#define FCR_XMITCLR   0x04
#define FCR_64        0x20
#define FCR_FIFO1     0x00
#define FCR_FIFO4     0x40
#define FCR_FIFO8     0x80
#define FCR_FIFO14    0xc0

#define COM1_BASE   0x3f8
#define COM1_IRQ    4
#define COM2_BASE   0x2f8
#define COM2_IRQ    3


/* Driver data sizes */
#define NPORTS    2
#define NBAUDS    25
#define RBUFFSZ   2048
#define SBUFFSZ   2048


/* Baud rates */
enum {
	BPS_50,     BPS_75,     BPS_110,    BPS_135,    BPS_150,
	BPS_300,    BPS_600,    BPS_1200,   BPS_1800,   BPS_2000,
	BPS_2400,   BPS_3600,   BPS_4800,   BPS_7200,   BPS_9600,
	BPS_19200,  BPS_28800,  BPS_31250,  BPS_38400,  BPS_57600,
	BPS_115200, BPS_230400, BPS_460800, BPS_921600, BPS_1500000
};


extern int serial_read(unsigned int pn, u8 *buff, u16 len, u16 timeout);

extern int serial_write(unsigned int pn, u8 *buff, u16 len);

extern void serial_init(u8 baud);

extern void serial_done(void);


#endif
