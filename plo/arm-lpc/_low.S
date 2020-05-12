#
# Phoenix-RTOS
#
# armplo - operating system loader for ARM7TDMI
#
# Low-level routines
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

        .text
        .arm

        .global irq_enable
        .global fiq_enable
        .global irq_disable
        .global fiq_disable
        .global low_getregs


        .func irq_enable
irq_enable:
				mrs		r0, cpsr
				bic		r0, r0, #0x80
				msr		cpsr_c, r0

				bx		lr
				nop
		.endfunc

        .func fiq_enable
fiq_enable:
				mrs		r0, cpsr
				bic		r0, r0, #0x40
				msr		cpsr_c, r0

				bx		lr
		.endfunc

        .func irq_disable
irq_disable:
				mrs		r0, cpsr
				orr		r0, r0, #0x80
				msr		cpsr_c, r0

				bx		lr
				nop
		.endfunc

        .func fiq_disable
fiq_disable:
				mrs		r0, cpsr
				orr		r0, r0, #0x40
				msr		cpsr_c, r0
				bx		lr
		.endfunc

		.func low_getregs
low_getregs:
				stmia	r0, {r0-r15}
				mrs		r2, cpsr
				str		r2, [r1]
				bx		lr
				nop
		.endfunc
		.end
