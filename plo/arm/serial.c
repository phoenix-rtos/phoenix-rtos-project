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

#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "timer.h"
#include "serial.h"


typedef struct{
	union{				/* 0x00 */
		vu32	rbr; 
		vu32 	thr;
		vu32	lsb;
	};
	union{					/* 0x04 */
		vu32	imr;
		vu32	msb;
	};
	union{					/* 0x08 */
		const vu32	iir;
		vu32	fcr;
	};
	vu32		lcr;		/* 0x0C */
	const vu32	pad1;	
	vu32		lsr;		/* 0x14 */
	const vu32	pad2[4];
	vu32		fdr;		/* 0x28 */
}* serial_regs_t;

typedef struct {
	u16 active;
	serial_regs_t cntl;
	u16 irq;
	u8	src;
	u8 rbuff[RBUFFSZ];
	u16 rb;
	u16 rp;
	u8 sbuff[SBUFFSZ];
	u16 sp;
	u16 se;
} serial_t;

serial_t serials[NPORTS];


int serial_isr(u16 irq, void *data)
{
	serial_t *serial = data;
	u8 iir;

	for (;;) {
		if ((iir = serial->cntl->iir) & IIR_IRQPEND)
			break;

		/* Receive */
		if ((iir & IIR_DR) == IIR_DR) {
			serial->rbuff[serial->rp] = serial->cntl->rbr;
			serial->rp = (++serial->rp % RBUFFSZ);
			if (serial->rp == serial->rb) {
				serial->rb = (++serial->rb % RBUFFSZ);
			}
		}
			
		/* Transmit */
		if ((iir & IIR_THRE) == IIR_THRE) {
			serial->sp = (++serial->sp % SBUFFSZ);
			if (serial->sp != serial->se)
				serial->cntl->thr = serial->sbuff[serial->sp];
		}
	}

	return 0;
}


int serial_read(unsigned int pn, u8 *buff, u16 len, u16 timeout)
{
	serial_t *serial;
	unsigned int l;
	unsigned int cnt;

	if (pn >= NPORTS)
		return ERR_ARG;
	serial = &serials[pn];
	if (!serial->active)
		return ERR_ARG;

	/* Wait for data */
	if (!timer_wait(timeout, TIMER_VALCHG, &serial->rp, serial->rb, NULL))
		return ERR_TIMEOUT;

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
	unsigned int l;
	unsigned int cnt;

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
	if (serial->se == serial->sp){
		serial->cntl->thr = serial->sbuff[serial->sp];
	}
	
	serial->se = ((serial->se + cnt) % SBUFFSZ);
	low_sti();

	return cnt;
}


void serial_initone(unsigned int pn, u32 base, u8 src, u16 irq, u16 speed, serial_t *serial)
{
	reg32 pins = (reg32) COM_PINSEL;

	serial->cntl = (serial_regs_t) base;
	serial->irq = irq;
	serial->src = src;
	serial->rb = 0;
	serial->rp = 0;
	serial->sp = 0;
	serial->se = 0;

	if (serial->cntl->iir == 0xff) {
		serial->active = 0;
		return;
	}
	serial->active = 1;

	*pins |= COM_PINVAL << (pn << 4);

	low_cli();

	low_irqinst(serial->src, serial->irq, serial_isr, (void *)serial);
	low_irqen(serial->irq, 1);
	
	/* Set speed */
	serial->cntl->lcr = LCR_DLAB;
	serial->cntl->lsb = speed;
	serial->cntl->msb = 0;

	/* Enable FIFO */
	serial->cntl->fcr = 0x07;

	/* Set data format */
	serial->cntl->lcr = LCR_D8N1;

	/* Set interrupt mask */
	serial->cntl->imr = IMR_THRE | IMR_DR;
	
	low_sti();
	return;
}


void serial_init(u16 speed)
{
	serial_initone(0, COM1_BASE, COM2_SRC, COM1_IRQ, speed, &serials[0]);
	serial_initone(1, COM2_BASE, COM2_SRC, COM2_IRQ, speed, &serials[1]);
	return;
}


void serial_done(void)
{
	unsigned int k;

	for (k = 0; k < NPORTS; k++) {
		low_cli();
		low_irqen(serials[k].irq, 0);
		low_irquninst(serials[k].irq);
		low_sti();
		return;
	}
}
