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

#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "timer.h"
#include "serial.h"


/* SCH311X controller */
#define SCH311X_PORT   0x2e
#define SCH311X_UART1  0x04
#define SCH311X_UART2  0x05
#define SCH3112        0x7c
#define SCH3114        0x7d
#define SCH3116        0x7f


typedef struct {
	u32 speed;
	u16 divisor;
	u8  mode;
} baud_t;


typedef struct {
	u16 active;
	u16 base;
	u16 irq;
	u8 rbuff[RBUFFSZ];
	u16 rb;
	u16 rp;
	u8 sbuff[SBUFFSZ];
	u16 sp;
	u16 se;
} serial_t;


baud_t bauds[NBAUDS] = {
	{ 50,      2304,  0 }, /* 0  - BPS_50 */
	{ 75,      1536,  0 }, /* 1  - BPS_75 */
	{ 110,     1047,  0 }, /* 2  - BPS_110 */
	{ 135,     857,   0 }, /* 3  - BPS_135 */
	{ 150,     768,   0 }, /* 4  - BPS_150 */
	{ 300,     384,   0 }, /* 5  - BPS_300 */
	{ 600,     192,   0 }, /* 6  - BPS_600 */
	{ 1200,    96,    0 }, /* 7  - BPS_1200 */
	{ 1800,    64,    0 }, /* 8  - BPS_1800 */
	{ 2000,    58,    0 }, /* 9  - BPS_2000 */
	{ 2400,    48,    0 }, /* 10 - BPS_2400 */
	{ 3600,    32,    0 }, /* 11 - BPS_3600 */
	{ 4800,    24,    0 }, /* 12 - BPS_4800 */
	{ 7200,    16,    0 }, /* 13 - BPS_7200 */
	{ 9600,    12,    0 }, /* 14 - BPS_9600 */
	{ 19200,   6,     0 }, /* 15 - BPS_19200 */
	{ 28800,   4,     0 }, /* 16 - BPS_28800 */
	{ 31250,   4,     1 }, /* 17 - BPS_31250 */
	{ 38400,   3,     0 }, /* 18 - BPS_38400 */
	{ 57600,   2,     0 }, /* 19 - BPS_57600 */
	{ 115200,  1,     0 }, /* 20 - BPS_115200 */
	{ 230400,  32770, 2 }, /* 21 - BPS_230400 */
	{ 460800,  32769, 2 }, /* 22 - BPS_460800 */
	{ 921600,  49153, 6 }, /* 23 - BPS_921600 */
	{ 1500000, 8193,  8 }  /* 24 - BPS_1500000 */
};


serial_t serials[NPORTS];


static u8 serial_SCH311Xinb(u16 port, u8 reg)
{
	low_outb(port, reg);
	return low_inb(port + 1);
}


static void serial_SCH311Xoutb(u16 port, u8 reg, u8 val)
{
	low_outb(port, reg);
	low_outb(port + 1, val);
}


static int serial_SCH311Xdetect(u16 port)
{
	int res;

	/* Enter configuration mode */
	low_outb(port, 0x55);

	/* Check device ID */
	res = serial_SCH311Xinb(port, 0x20);

	switch (res) {
	case SCH3112:
	case SCH3114:
	case SCH3116:
		break;

	default:
		res = -1;
	}

	/* Exit configuration mode */
	low_outb(port, 0xaa);

	return res;
}


static int serial_SCH311X_UARTsetmode(u16 port, u8 uart, u8 mode)
{
	/* Enter configuration mode */
	low_outb(port, 0x55);

	/* Select logical UART device */
	serial_SCH311Xoutb(port, 0x07, uart);
	/* Set mode */
	serial_SCH311Xoutb(port, 0xf0, mode);

	/* Exit configuration mode */
	low_outb(port, 0xaa);

	return 0;
}


static int serial_isr(u16 irq, void *data)
{
	serial_t *serial = data;
	u8 iir;

	for (;;) {
		if ((iir = low_inb(serial->base + REG_IIR)) & IIR_IRQPEND)
			break;

		/* Receive */
		if ((iir & IIR_DR) == IIR_DR) {
			while (low_inb(serial->base + REG_LSR) & LSR_DR) {
				serial->rbuff[serial->rp] = low_inb(serial->base + REG_RBR);
				serial->rp = (++serial->rp % RBUFFSZ);
				if (serial->rp == serial->rb) {
					serial->rb = (++serial->rb % RBUFFSZ);
				}
			}
		}

		/* Transmit */
		if ((iir & IIR_THRE) == IIR_THRE) {
			serial->sp = (++serial->sp % SBUFFSZ);
			if (serial->sp != serial->se)
				low_outb(serial->base + REG_THR, serial->sbuff[serial->sp]);
		}
	}
	return IRQ_HANDLED;
}


int serial_read(unsigned int pn, u8 *buff, u16 len, u16 timeout)
{
	serial_t *serial;
	u16 l, cnt;

	if (pn >= NPORTS)
		return ERR_ARG;
	serial = &serials[pn];
	if (!serial->active)
		return ERR_ARG;

	/* Wait for data */
	if (!timer_wait(timeout, TIMER_VALCHG, &serial->rp, serial->rb))
		return ERR_SERIAL_TIMEOUT;

	low_cli();

	if (serial->rp > serial->rb)
		l = min(serial->rp - serial->rb, len);
	else
		l = min(RBUFFSZ - serial->rb, len);

	low_memcpy(buff, &serial->rbuff[serial->rb], l);
	cnt = l;
	if ((len > l) && (serial->rp < serial->rb)) {
		low_memcpy(buff + l, &serial->rbuff[0], min(len - l, serial->rp));
		cnt += min(len - l, serial->rp);
	}
	serial->rb = ((serial->rb + cnt) % RBUFFSZ);

	low_sti();

	return cnt;
}


int serial_write(unsigned int pn, u8 *buff, u16 len)
{
	serial_t *serial;
	u16 l, cnt;

	if (pn >= NPORTS)
		return ERR_ARG;
	serial = &serials[pn];
	if (!serial->active)
		return ERR_ARG;

	low_cli();

	if (serial->sp > serial->se)
		l = min(serial->sp - serial->se, len);
	else
		l = min(SBUFFSZ - serial->se, len);

	low_memcpy(&serial->sbuff[serial->se], buff, l);
	cnt = l;
	if ((len > l) && (serial->se >= serial->sp)) {
		low_memcpy(serial->sbuff, buff + l, min(len - l, serial->sp));
		cnt += min(len - l, serial->sp);
	}

	/* Initialize sending process */
	if (serial->se == serial->sp)
		low_outb(serial->base, serial->sbuff[serial->sp]);
	serial->se = ((serial->se + cnt) % SBUFFSZ);

	low_sti();

	return cnt;
}


static void serial_initone(unsigned int pn, u16 base, u16 irq, u8 baud, serial_t *serial)
{
	serial->base = base;
	serial->irq = irq;
	serial->rb = 0;
	serial->rp = 0;
	serial->sp = (u16)-1;
	serial->se = 0;

	if (low_inb(serial->base + REG_IIR) == 0xff) {
		serial->active = 0;
		return;
	}
	serial->active = 1;

	low_cli();

	low_irqinst(serial->irq, serial_isr, (void *)serial);
	low_maskirq(serial->irq, 0);

	/* Set speed */
	low_outb(serial->base + REG_LCR, LCR_DLAB);
	low_outb(serial->base + REG_LSB, bauds[baud].divisor & 0xff);
	low_outb(serial->base + REG_MSB, bauds[baud].divisor >> 8);

	/* Set data format */
	low_outb(serial->base + REG_LCR, LCR_D8N1);

	/* Enable and configure the FIFOs */
	low_outb(serial->base + REG_FCR, FCR_ENABLE | FCR_RCVRCLR | FCR_XMITCLR | FCR_FIFO8 | FCR_64);

	/* Enable hardware interrupts */
	low_outb(serial->base + REG_MCR, MCR_OUT2);

	/* Set interrupt mask */
	low_outb(serial->base + REG_IMR, IMR_THRE | IMR_DR);

	low_sti();

	return;
}


void serial_init(u8 baud)
{
	if (serial_SCH311Xdetect(SCH311X_PORT) >= 0) {
		serial_SCH311X_UARTsetmode(SCH311X_PORT, SCH311X_UART1, bauds[baud].mode);
		serial_SCH311X_UARTsetmode(SCH311X_PORT, SCH311X_UART2, bauds[baud].mode);
	}
	serial_initone(0, COM1_BASE, COM1_IRQ, baud, &serials[0]);
	serial_initone(1, COM2_BASE, COM2_IRQ, baud, &serials[1]);

	return;
}


void serial_done(void)
{
	unsigned int k;

	for (k = 0; k < NPORTS; k++) {
		low_cli();
		low_maskirq(serials[k].irq, 1);
		low_irquninst(serials[k].irq);
		low_sti();
		return;
	}
}
