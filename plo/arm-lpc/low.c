/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Low-level routines
 *
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
#include "iap.h"
#include "timer.h"


#define CR		0x0D
#define CSI		"\x1B["
#define ESC		"\x1B"

#define GPIO1		((gpio_t)		 0xE0028010)
#define PINSEL		((reg32)		 0xE002C000)
#define COM0_BASE	((serial_regs_t) 0xe000c000)
#define EXTINT_BASE ((extint_t)		 0xE01FC140)
#define RTC_BASE	((rtc_t)		 0xE0024000)
#define PCON		((reg32)		 0xE01FC0C0)
#define PCONP		((reg32)		 0xE01FC0C4)


typedef volatile struct{
	const vu32 irq_stat;
	const vu32 fiq_stat;
	const vu32 int_stat;
	vu32 int_sel;
	vu32 int_enable;
	vu32 int_enclr;
	vu32 soft_int;
	vu32 soft_intclr;
	vu32 protection;
} *vic_cntl_t;

typedef struct {
	vic_cntl_t vic_cntl;
	vu32 *irq_no;
	vu32 *irq_cntl;
	int (*irq_isr[16])(u16, void*);
	void* irq_data[16];
} vic_data_t;

typedef struct {
	vu32 pin;
	vu32 set;
	vu32 dir;
	vu32 clr;
}* gpio_t;

typedef struct {
	vu32 eint;
	vu32 wake;
	vu32 mode;
	vu32 polar;
}* extint_t;

typedef struct {
	vu32 sec;
	vu32 min;
	vu32 hour;
	vu32 dom;
	vu32 dow;
	vu32 doy;
	vu32 month;
	vu32 year;
} datetime_t;

typedef struct {
	vu32		ilr;
	const vu32	ctc;
	vu32		ccr;
	vu32		ciir;
	vu32		amr;
	vu32		ctime[3];
	datetime_t	now;
	u8			pad[0x20];
	datetime_t	alarm;
	vu32		preint;
	vu32		prefrac;
}* rtc_t;


typedef struct{
	union{				/* 0x00 */
		vu32	rbr;
		vu32 	thr;
		vu32	lsb;
	};
	union{				/* 0x04 */
		vu32	imr;
		vu32	msb;
	};
	vu32		fcr;		/* 0x08 */
	vu32		lcr;		/* 0x0C */
	const vu32	pad1;
	vu32		lsr;		/* 0x14 */
	const vu32	pad2[4];
	vu32		fdr;		/* 0x28 */
}* serial_regs_t;

struct {
	reg8 in;
	reg8 out;
	reg8 status;
} console;

time_st		now = ((volatile time_st) 0xE0024020);
syspage_t	syspage;
vic_data_t	vic_data = {(vic_cntl_t) 0xFFFFF000, (reg32) 0xFFFFF100,(reg32) 0xFFFFF200} ;
u16		_plo_timeout = 3;
char		_plo_command[CMD_SIZE] = "load com1; go!";
pdata_t		*pdata_ptr = (pdata_t *)PDATA_ADDR;


extern void plostd_printf(char attr, char *, ...);


void low_memcpy(char *dst, char *src, unsigned int l)
{
	int i;
 	for(i = 0; i<l ;i++){
	 	*(dst+i) = *(src+i);
	}
}


/* Serial initialisation */
void low_consoleInit()
{
	serial_regs_t serial = COM0_BASE;
	*PINSEL |= 0x00000005;			/* Enable RxD0 and TxD0						*/
	serial->fcr = 0x07; /* Enable FIFO */
	serial->lcr = 0x83;  /* 8 bits, no Parity, 1 Stop bit */
	serial->lsb = 6; /* 115200 Baud Rate @ 15MHz VPB Clock */
  	serial->fdr = 0xe5;
	serial->lcr = 0x03;  /* DLAB = 0 */

	console.out = (reg8) &serial->thr;
	console.in =  (reg8) &serial->rbr;
	console.status = (reg8) &serial->lsr;

	plostd_puts(ATTR_LOADER, ESC "c" CSI "7h" CSI "2J" CSI "3;25r" CSI "1;1H");
}

int eint_handler(u16 irq, void* data)
{
	plostd_printf(ATTR_DEBUG,"\nExtInt test=%d\n", RTC_BASE->now.sec);

	EXTINT_BASE->eint = 0x2;
	return 0;
}

void low_extintInit(void)
{
	extint_t ext = EXTINT_BASE;
	/* Enable P0.14 EINT1 pin function: PINSEL0[29:28] = 10b */
	*PINSEL |= (2 << 28);

	/* Make EINT1 falling edge-sensitive
	 * (level sensitive increments the LED count too fast)
	 */
	ext->mode = 2;
	ext->polar = 0;

	/* Clear register after mode change */
	ext->eint = 0xf;

	low_cli();
	low_irqinst(15, 15, eint_handler, NULL);
	low_irqen(15, 1);
	low_sti();

}


void low_rtcInit(void)
{
	rtc_t rtc = RTC_BASE;
	rtc->ilr=0x3; /* Clear the Interrupt Location Register */
	rtc->ccr = 0x11; /* start RTC */
	*PCONP=0x1815BE; /* Disconnect pclk to RTC */
}


void low_putc(char attr, char ch)
{
  if (ch == '\n')  {
    while (!(*console.status & 0x20));
    *console.out = CR;                          /* output CR */
  }

  while (!(*console.status & 0x20));
  *console.out = ch;
}


void low_getc(char *c, char *sc)
{
  while (!(*console.status & 0x01));
  *c = *console.in;
  if(*c == 0x1b){
		while (!(*console.status & 0x01));
  		*c = *console.in;
		if(*c != '[' && *c != 'O')
			return;
		*c = 0;
		while (!(*console.status & 0x01));
  		*sc = *console.in;
  }

}


int low_keypressed()
{
	return *console.status & 0x01;
}


void low_init()
{

 	GPIO1->dir = 0x00FF0000; /* set LED output */

	low_rtcInit();
	low_consoleInit();
	low_extintInit();
	low_dbg(0x00);

	if(pdata_ptr->magic == PDATA_MAGIC){
		_plo_timeout = pdata_ptr->timeout;
		low_memcpy(_plo_command, pdata_ptr->command, CMD_SIZE);
	}
}


void low_irqinst(u8 src, u16 irq, int (*isr)(u16, void*), void* data)
{
	if(src > 31 || irq > 15) return;

	vic_data.irq_no[irq] = irq;
	vic_data.irq_isr[irq] = isr;
	vic_data.irq_data[irq] = data;
	vic_data.irq_cntl[irq] = 0x20 | src;
	vic_data.vic_cntl->int_sel &= ~(1 << src);
}


void low_irqen(u16 irq, u8 enable)
{
	u8 src = (u8) vic_data.irq_cntl[irq] & 0x1f;

	if(enable)
		vic_data.vic_cntl->int_enable = 1 << src;
	else
		vic_data.vic_cntl->int_enclr = 1 << src;
}


void low_irquninst(u16 irq)
{
	vic_data.irq_cntl[irq] = 0;
	vic_data.irq_data[irq] = NULL;
	vic_data.irq_isr[irq] = NULL;
}


void low_done()
{
	low_cli();
	low_irqen(15, 0);
	low_irquninst(15);
	low_sti();
}


void low_sti(){
	irq_enable();
}


void low_cli(){
	irq_disable();
}


void low_dbg(u8 state)
{
	GPIO1->clr =  0x00FF0000;
	GPIO1->set = ((int) state) << 0x10;
}
