/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Loader startup code
 *
 * Copyright 2012, 2014, 2015 Phoenix Systems
 * Copyright 2006 Radoslaw F. Wawrzusiak
 * Author: Radoslaw F. Wawrzusiak, Jacek Popko, Katarzyna Baranowska
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

#define __ASSEMBLY__
/* #include "cpu.h" */

#define USR_MODE	0x10
#define FIQ_MODE	0x11
#define IRQ_MODE	0x12
#define SVC_MODE	0x13	/* reset mode */
#define ABT_MODE	0x17
#define UND_MODE	0x1B
#define SYS_MODE	0x1F
#define MODE_MASK	0x1F
#define NO_ABORT	0x100	/* mask to disable Abort Exception */
#define NO_IRQ		0x80	/* mask to disable IRQ */
#define NO_FIQ		0x40	/* mask to disable FIQ */
#define	NO_INT		(NO_IRQ | NO_FIQ)	/* mask to disable IRQ and FIQ */
#define THUMB_STATE	0x20

/** Data cache line size
 * Particularly useful when allocating data structure which are subject
 * to cache invalidation. As the operation is performed on multiple
 * of line size, such data structures should be aligned to this size
 * and occupy complete number of lines. */
#define SIZE_CACHE_LINE	32

		/* Stack sizes */
#define FIQ_STACK_SIZE 0x00000020 /* 32x32-bit words */
#define IRQ_STACK_SIZE 0x00000080 /* 32x32-bit words */
#define SVC_STACK_SIZE 0x00000040
#define ABT_STACK_SIZE 0x00000040 /* 32x32-bit words */
#define UND_STACK_SIZE 0x00000040
#define SYS_STACK_SIZE 0x00000080 /* 256x32-bit words */


#
# Startup code
#
		.global vectors
		.section .text, "ax"
		.arm
		.align 5

# Interrupt and exception vectors remapped to address 0
vectors:
        ldr     pc, reset_addr         
		ldr     pc, undef_addr
		ldr     pc, swi_addr
		ldr     pc, pabt_addr
		ldr     pc, dabt_addr
		nop			/* Reserved Vector */
		ldr		pc, irq_addr
		ldr     pc, fiq_addr

reset_addr:	.word   _start
undef_addr:	.word   Undef_Handler
swi_addr:	.word   SWI_Handler
pabt_addr:	.word   PAbt_Handler
dabt_addr:	.word   DAbt_Handler
irq_addr:	.word   FIQ_Handler
fiq_addr:	.word   FIQ_Handler
			.word	0
# End of remapped area


		.section .boot, "ax"
		.global _start
		.func _start
_start:
		cpsid	aif, #SYS_MODE
		ldr		sp, =STACK_START

bss_clear:
		mov	r0, #0
		ldr	r1, =_fbss_phy
		ldr	r2, =_ebss_phy
_bssLoop:
		cmp	r1, r2
		strlo	r0, [r1], #4
		blo	_bssLoop

		/* Install interrupts vectors */
		ldr		r3, =vectors
		mcr		15, 0, r3, c12, c0, 0

		blx		low_clocksInit
		blx		low_ddrInit

_stack_init:
		mov r0, sp
		/* FIQ mode stack */
		msr	CPSR_c, #FIQ_MODE|NO_INT
		mov	sp, r0
		sub	r0, r0, #FIQ_STACK_SIZE

		/* IRQ mode stack */
		msr	CPSR_c, #IRQ_MODE|NO_INT
		mov	sp, r0
		sub	r0, r0, #IRQ_STACK_SIZE

		/* Supervisor mode stack */
		msr	CPSR_c, #SVC_MODE|NO_INT
		mov	sp, r0
		sub	r0, r0, #SVC_STACK_SIZE

		/* Undefined mode stack */
		msr	CPSR_c, #UND_MODE|NO_INT
		mov	sp, r0
		sub	r0, r0, #UND_STACK_SIZE

		/* Abort mode stack */
		msr	CPSR_c, #ABT_MODE|NO_INT
		mov	sp, r0
		sub	r0, r0, #ABT_STACK_SIZE

		/* System mode stack */
		msr	CPSR_c, #SYS_MODE |NO_INT
		mov	sp, r0

		#  Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
		sub     sl, sp, #SYS_STACK_SIZE
		
		ldr		lr, =plo_init
		bx		lr
1:
		b 	1b

		.endfunc
