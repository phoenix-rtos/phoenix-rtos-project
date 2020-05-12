/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * Exception handlers
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
#include "low.h"
#include "plostd.h"

extern u32 STACK_START;

void FIQ_Handler(void)		__attribute__ ((interrupt("FIQ")));
void PAbt_Handler(void)		__attribute__ ((interrupt("ABORT")));
void DAbt_Handler(void)		__attribute__ ((interrupt("ABORT")));
void SWI_Handler(void)		__attribute__ ((interrupt("SWI")));
void Undef_Handler(void)	__attribute__ ((interrupt("UNDEF")));

void memory_dump(u32, u32);


void Undef_Handler(void)
{
	int i, ip = 0;
	psr_t cpsr, spsr;
	int regs[16];

	__asm__ (
				"sub %0, lr, #4\n\t"
				"mrs %1, cpsr\n\t"
				"mrs %2, spsr\n\t"
				"stmia %3, {r0-r15}^"
				: "=&r" (ip), "=&r" (cpsr.all), "=&r" (spsr.all)
				: "r" (regs)
			);

	plostd_printf(ATTR_DEBUG, "\nUndefined Instruction @ %p\n", ip);
	plostd_printf(ATTR_DEBUG, "User registers:\n");
	for(i=0; i<5; i++){
		plostd_printf(ATTR_DEBUG, "R%d\t=%p\tR%d\t=%p\tR%d\t=%p\n", i, regs[i], i+5, regs[i+5], i+10, regs[i+10]);
	}
	plostd_printf(ATTR_DEBUG, "CPSR\t:%p\tSPSR\t:%p, i:%x\n", cpsr.all, spsr.all, spsr.i_bit);

	plostd_printf(ATTR_DEBUG, "Stack Dump:\n");
	memory_dump(regs[13], STACK_START);

	low_dbg(1);
	for(;;);
}


void SWI_Handler(void)
{
	low_dbg(0x22);
}


void PAbt_Handler(void)
{
	int i, cpsr, spsr, ip = 0;
	int regs[16];

	__asm__ (
				"sub %0, lr, #4\n\t"
				"mrs %1, cpsr\n\t"
				"mrs %2, spsr\n\t"
				"stmia %3, {r0-r15}^"
				: "=&r" (ip), "=&r" (cpsr), "=&r" (spsr)
				: "r" (regs)
			);

	plostd_printf(ATTR_DEBUG, "\nPrefetch Abort => instr @ %p\n", ip);
	plostd_printf(ATTR_DEBUG, "User registers:\n");
	for(i=0; i<5; i++){
		plostd_printf(ATTR_DEBUG, "R%d\t=%p\tR%d\t=%p\tR%d\t=%p\n", i, regs[i], i+5, regs[i+5], i+10, regs[i+10]);
	}
	plostd_printf(ATTR_DEBUG, "CPSR\t:%p\tSPSR\t:%p\n", cpsr, spsr);

	plostd_printf(ATTR_DEBUG, "Stack Dump:\n");
	memory_dump(regs[13], STACK_START);

	low_dbg(3);
	for(;;);
}


void DAbt_Handler(void)
{
	int regs[16];
	int i, cpsr, spsr, ip = 0;

	__asm__ (
				"sub %0, lr, #4\n\t"
				"mrs %1, cpsr\n\t"
				"mrs %2, spsr\n\t"
				"stmia %3, {r0-r15}^"
				: "=&r" (ip), "=&r" (cpsr), "=&r" (spsr)
				: "r" (regs)
			);

	plostd_printf(ATTR_DEBUG, "\nData Abort => instr @ %p\n", ip);
	plostd_printf(ATTR_DEBUG, "User registers:\n");
	for(i=0; i<5; i++){
		plostd_printf(ATTR_DEBUG, "R%d\t=%p\tR%d\t=%p\tR%d\t=%p\n", i, regs[i], i+5, regs[i+5], i+10, regs[i+10]);
	}
	plostd_printf(ATTR_DEBUG, "CPSR\t:%p\tSPSR\t:%p\n", cpsr, spsr);

	plostd_printf(ATTR_DEBUG, "Stack Dump:\n");
	memory_dump(regs[13], STACK_START);

	low_dbg(4);
	for(;;);
}


void FIQ_Handler(void)
{
	low_dbg(7);
	for(;;);
}


void memory_dump(u32 addr, u32 max)
{
	u32 x, y, iaddr;
	u8 b;

	for (y = 0; y < 8; y++) {
		plostd_printf(ATTR_DEBUG, "%p   ", addr);
		iaddr = addr;

		/* Print byte values */
		for (x = 0; x < 16; x++) {
			if(addr >= max){
				plostd_printf(ATTR_DEBUG, "XX ");
				addr++;
				continue;
			}
			b = *((u8 *)addr);
			if (b & 0xf0)
				plostd_printf(ATTR_DEBUG, "%x ", b);
			else
				plostd_printf(ATTR_DEBUG, "0%x ", b);

			addr++;
		}
		plostd_printf(ATTR_DEBUG, "  ");

		/* Print ASCII representation */
		addr = iaddr;

		for (x = 0; x < 16; x++) {
			if(addr >= max){
				plostd_printf(ATTR_DEBUG, ".");
				addr++;
				continue;
			}
			b = *((u8 *)addr);
			if ((b <= 32) || (b > 127))
				plostd_printf(ATTR_DEBUG, ".", b);
			else
				plostd_printf(ATTR_DEBUG, "%c", b);

			addr++;
		}
		plostd_printf(ATTR_DEBUG, "\n");
		if(addr >= max) break;
	}
}
