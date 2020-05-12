;
; Phoenix-RTOS
;
; plo - operating system loader
;
; Interrupt stubs
;
; Copyright 2012 Phoenix Systems
; Copyright 2005 Pawel Pisarczyk
;
; This file is part of Phoenix-RTOS
;
; %LICENSE%
;

MACRO IRQL
.globl ?1
?1:
MEND

MACRO IRQ
	cli
	push #?1
	jmp __irq_stub
MEND

.data

extern _irqdata

.text

extern _low_irqdispatch

IRQL __irq0
IRQ 0

IRQL __irq1
IRQ 1

IRQL __irq2
IRQ 2

IRQL __irq3
IRQ 3

IRQL __irq4
IRQ 4

IRQL __irq5
IRQ 5

IRQL __irq6
IRQ 6

IRQL __irq7
IRQ 7

IRQL __irq8
IRQ 8

IRQL __irq9
IRQ 9

IRQL __irq10
IRQ 10

IRQL __irq11
IRQ 11

IRQL __irq12
IRQ 12

IRQL __irq13
IRQ 13

IRQL __irq14
IRQ 14

IRQL __irq15
IRQ 15

.globl __irq_stub
__irq_stub:
	cli
	push bx
	mov bx, sp
	seg ss
	xchg ax, 2[bx]
	push cx
	push dx
	push si
	push di
	push bp
	push ds
	push es

	mov cx, ax
	mov ax, cs
	mov es, ax
	mov ds, ax

	push cx
	call _low_irqdispatch
	cli
	pop cx
	or ax, ax
	jnz __irqstub_default

	mov al, #0x20
	out #0x20, al
	cmp cx, #8
	jc __irqstub_end
	out #0xa0, al

__irqstub_end:
	pop es
	pop ds
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	iret

__irqstub_default:
	shl cx, #1
	mov si, cx
	add si, #_irqdata
	seg ds
	mov ax, word ptr [si]
	seg ds
	mov bx, word ptr 32[si]

	pop es
	pop ds
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	push bp
	mov bp, sp
	seg ss
	xchg bx, 2[bp]
	seg ss
	xchg ax, 4[bp]
	pop bp
	retf
