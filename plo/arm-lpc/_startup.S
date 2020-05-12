#
# Phoenix-RTOS
#
# armplo - operating system loader for ARM7TDMI
#
# Startup code
#
# Copyright 2006	Radoslaw F. Wawrzusiak
#
# This file is part of Phoenix-RTOS.
#
# Phoenix-RTOS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Phoenix-RTOS kernel is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Phoenix-RTOS kernel; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

	/* Constants (used as immediate values) */
		/* Processor modes (see pA2-11 ARM-ARM) */
		.equ	USR_MODE,		0x10
		.equ	FIQ_MODE,		0x11
		.equ	IRQ_MODE,		0x12
		.equ	SVC_MODE,		0x13 /* reset mode */
		.equ	ABT_MODE,		0x17
		.equ	UND_MODE,		0x1B
		.equ	SYS_MODE,		0x1F

		/* Stack sizes */
		.equ	FIQ_STACK_SIZE,	0x00000080 /* 32x32-bit words */
		.equ	IRQ_STACK_SIZE,	0x00000400
		.equ	SVC_STACK_SIZE,	0x00000080
		.equ	ABT_STACK_SIZE,	0x00000010 /* 4x32-bit words */
		.equ	UND_STACK_SIZE,	0x00000010
		.equ	SYS_STACK_SIZE,	0x00000400 /* 256x32-bit words */

		/* CPSR interrupt disable bits */
		.equ IRQ_DISABLE, (1 << 7)
		.equ FIQ_DISABLE, (1 << 6)

# PLL CONSTANTS

/* Constants (used as immediate values) */
		.equ	PLLCON_OFFSET,	0x0
		.equ	PLLCFG_OFFSET,	0x4
		.equ	PLLSTAT_OFFSET,	0x8
		.equ	PLLFEED_OFFSET,	0xC
		.equ	PLLCON_PLLE,	(1 << 0)
		.equ	PLLCON_PLLC,	(1 << 1)
		.equ	PLLSTAT_PLOCK,	(1 << 10)
		.equ	PLLFEED1,		0xAA
		.equ	PLLFEED2,		0x55
		.equ	PLLCFG_VALUE,	0x24

# MAM setup constants

		/* Constants (used as immediate values) */
		.equ	MAMCR_OFFSET,	0x0
		.equ	MAMTIM_OFFSET,	0x4
		.equ	MAMCR_VALUE,	0x2 /* fully enabled */
		.equ	MAMTIM_VALUE,	0x4 /* fetch cycles */

# Starupt Code must be linked first at Address at which it expects to run.

		.global _etext
		.global _data
		.global _edata
		.global _bss
		.global _ebss
		.global	text
        .global	etext
        .global STACK_START

        .text
        .arm

        .global _start
        .func   _start
_start:


# Exception Vectors
#  Mapped to Address 0.
#  Absolute addressing mode must be used.
#  Dummy Handlers are implemented as infinite loops which can be modified.

Vectors:        ldr     pc, Reset_Addr
                ldr     pc, Undef_Addr
                ldr     pc, SWI_Addr
                ldr     pc, PAbt_Addr
                ldr     pc, DAbt_Addr
                nop								/* Reserved Vector */
				ldr     pc, irq_addr
                ldr     pc, FIQ_Addr

Reset_Addr:     .word   Reset_Handler
Undef_Addr:     .word   Undef_Handler
SWI_Addr:       .word   SWI_Handler
PAbt_Addr:      .word   PAbt_Handler
DAbt_Addr:      .word   DAbt_Handler
                .word   0                      /* Reserved Address */
irq_addr:       .word   irq_handler
FIQ_Addr:       .word   FIQ_Handler


# PLL DATA
PLLBASE:		.word	0xE01FC080

# MAM setup data
MAMBASE:		.word	0xE01FC000

# STACK DATA
STACK_START:	.word	0x40007fe0

# Reset Handler

Reset_Handler:

# INITIALISE PLL
pll_init:
				/* Use r0 for indirect addressing */
				ldr		r0, PLLBASE

				/* PLLCFG = PLLCFG_VALUE */
				mov		r3, #PLLCFG_VALUE
				str		r3, [r0, #PLLCFG_OFFSET]

				/* PLLCON = PLLCON_PLLE */
				mov		r3, #PLLCON_PLLE
				str		r3, [r0, #PLLCON_OFFSET]

				/* PLLFEED = PLLFEED1, PLLFEED2 */
				mov		r1, #PLLFEED1
				mov		r2, #PLLFEED2
				str		r1, [r0, #PLLFEED_OFFSET]
				str		r2, [r0, #PLLFEED_OFFSET]

				/* while ((PLLSTAT & PLLSTAT_PLOCK) == 0); */
pll_loop:
				ldr		r3, [r0, #PLLSTAT_OFFSET]
				tst		r3, #PLLSTAT_PLOCK
				beq		pll_loop

				/* PLLCON = PLLCON_PLLC|PLLCON_PLLE */
				mov		r3, #PLLCON_PLLC|PLLCON_PLLE
				str		r3, [r0, #PLLCON_OFFSET]

				/* PLLFEED = PLLFEED1, PLLFEED2 */
				str		r1, [r0, #PLLFEED_OFFSET]
				str		r2, [r0, #PLLFEED_OFFSET]

# MAM setup
mam_init:
				/* Use r0 for indirect addressing */
				ldr		r0, MAMBASE

				/* MAMTIM = MAMTIM_VALUE */
				mov		r1, #MAMTIM_VALUE
				str		r1, [r0, #MAMTIM_OFFSET]

				/* MAMCR = MAMCR_VALUE */
				mov		r1, #MAMCR_VALUE
				str		r1, [r0, #MAMCR_OFFSET]


# Setup Stack for each mode
				ldr		r0, STACK_START

				/* FIQ mode stack */
				msr		CPSR_c, #FIQ_MODE|IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0
				sub		r0, r0, #FIQ_STACK_SIZE

				/* IRQ mode stack */
				msr		CPSR_c, #IRQ_MODE|IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0
				sub		r0, r0, #IRQ_STACK_SIZE

				/* Supervisor mode stack */
				msr		CPSR_c, #SVC_MODE|IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0
				sub		r0, r0, #SVC_STACK_SIZE

				/* Undefined mode stack */
				msr		CPSR_c, #UND_MODE|IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0
				sub		r0, r0, #UND_STACK_SIZE

				/* Abort mode stack */
				msr		CPSR_c, #ABT_MODE|IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0
				sub		r0, r0, #ABT_STACK_SIZE

				/* System mode stack */
				msr		CPSR_c, #SYS_MODE |IRQ_DISABLE|FIQ_DISABLE
				mov		sp, r0

				/* Leave the processor in system mode */


#  Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
                SUB     SL, SP, #SYS_STACK_SIZE


# Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel


# Clear .bss section (Zero init)
                MOV     R0, #0
                LDR     R1, =_bss
                LDR     R2, =_ebss
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI


# Enter the C code
                b       plo_init

loop:			b		loop

        .endfunc

#
# IRQ handler
#
        .func irq_handler
irq_handler:
				sub		lr, lr, #4
				stmfd	sp!, {r0-r12, lr}

				mvn		r11, #4032				/* prepare VICVectAddr address	*/
				sub		r11, r11, #15			/* 0xffff ff030					*/
				ldr		r2, ih_itab				/* get isr table pointer		*/
				ldr		r1, ih_dtab				/* get isr data table pointer	*/
				ldr		r0, [r11]				/* get irq number (isr arg 1)	*/
				ldr		r3, [r2, r0, lsl #2]	/* get isr address				*/
				ldr		r1, [r1, r0, lsl #2]	/* get isr data (isr arg 2)		*/

				add		lr, pc, #4				/* call isr(irq_no, irq_data)	*/
				bx		r3

				mov	r2, #255					/* VICVectAddr = 0xff			*/
				str	r2, [r11]					/* update priority hardware		*/
				ldmfd	sp!, {r0-r12, pc}^		/* return from interrupt		*/
ih_itab:		.word	vic_data+4+4+4
ih_dtab:		.word	vic_data+4+4+4+64

		.endfunc

        .data
        .global SYSPAGE
SYSPAGE:		.word	syspage
        .end
