;
; Phoenix-RTOS
;
; plo - operating system loader
;
; Loader startup code
;
; Copyright 2012, 2020 Phoenix Systems
; Copyright 2001, 2005, 2006 Pawel Pisarczyk
; Author: Pawel Pisarczyk, Lukasz Kosinski
;
; This file is part of Phoenix-RTOS.
;
; %LICENSE%
;

.text

.extern _plo_init


; Loader entry point
.globl _main
_main:
	br __plo_start


; Loader variables

.globl __plo_baid
__plo_baid:
	db 00

.globl __plo_offs
__plo_offs:
	dw 0001

.globl __plo_size
__plo_size:
	dw 50

.globl __plo_timeout
__plo_timeout:
	dw 0001

.globl __plo_command
__plo_command:
.ascii "load hd0; go!\0"
.zerow 60

__plo_digitsh:
.ascii "0123456789abcdef"

__plo_pos:
	dw 0


; Loader initialization
.globl __plo_start
__plo_start:
	cli
	mov ax, #0x7c0
	mov ds, ax
	mov es, ax
	mov ss, ax

; Loader stack is located below disk cache offset
	mov sp, #0x9000

; Store boot device number and reload CS
	mov __plo_baid, dl
	sti
	push ax
	mov ax, #__plo_boot
	push ax
	retf

; Load all loader parts into memory
__plo_boot:
	xor dx, dx
	mov dl, __plo_baid
	mov si, __plo_offs
	mov di, #0x200
	mov cx, __plo_size
	call __plo_read
	test ax, ax
	jnz __plo_end

; When loader exist in memory call its high level part
	call _plo_init

; Print end signature
__plo_end:
	mov ax, #0x55aa
	call __plo_debug

; Reboot after exit
	mov ax, #0xffff
	push ax
	xor ax, ax
	push ax
	retf


; Function reads sectors from disk to buffer given by di
.globl __plo_read
__plo_read:

; Calculate last buffer offset
	mov ax, di
	shl cx, #9
	add ax, cx

	push ax     ; Last offset
	push di     ; Destination
	push dx     ; Device

__plo_read_l0:

; Determine boot device geometry
	push es
	push si
	mov ah, #0x08
	int 0x13
	pop si
	pop es

; Store number of cylinders and sectors in registers cx - sectors, bp - heads
	and cx, #0x3f
	mov bp, dx
	shr bp, #8
	inc bp

; Calculate starting sector
	mov ax, si
	xor dx, dx
	div cx
	inc dx
	push dx

; Calculate number of sectors remaining on track
	xchg ax, cx
	sub ax, dx
	inc ax
	push ax

; Calculate number of sectors remaining to read
	mov bx, 6[esp]    ; load current destination address
	mov ax, 8[esp]    ; load last offset
	sub ax, bx
	shr ax, #9        ; calculate number of remaining sectors

	pop bx
	cmp ax, bx        ; load minimal number of sectors
	jl __plo_read_l1
	mov ax, bx
__plo_read_l1:
	push ax

; Calculate starting cylinder and head
	xchg ax, cx
	xor dx, dx
	div bp
	mov cx, ax
	ror ch, #2
	xchg ch, cl

; Get starting head
	shl dx, #8

; Get number of sectors to read
	pop ax
	mov ah, #2

; Get starting sector
	pop bp
	or cx, bp

; Get device
	pop bx
	mov dl, bl

; Get offset
	pop bx
	push ax
	int 0x13
	jc __plo_read_l3
	pop ax

; Calculate current memory and disk offsets
	mov ah, #0
	add si, ax
	shl ax, #9
	add ax, bx

	pop cx
	push cx
	push ax
	push dx
	cmp ax, cx
	jl __plo_read_l0

	pop dx
	pop ax
	mov ax, #0
	br __plo_read_l4

__plo_read_l3:
	pop ax
	mov ax, #1

__plo_read_l4:
	add sp, #2
	ret


; Function writes MBR sector to disk
.globl __plo_save
__plo_save:
	push bx
	push cx
	push dx

	xor bx, bx
	mov al, #1
	mov ah, #3
	mov cx, #1
	mov dh, #0
	mov dl, __plo_baid
	int 0x13
	jc __plo_save_l0
	xor ax, ax

__plo_save_l0:
	pop dx
	pop cx
	pop bx
	ret


; Function prints value given in ax register as hexadecimal string
__plo_debug:
	push es
	push ax
	push bx
	push cx
	push di

	mov bx, ax
	mov ax, #0xb800
	mov es, ax
	mov ax, bx
	mov bx, __plo_pos
	mov cx, #4

__plo_debug_l0:
	rol ax, #4
	push ax
	and ax, #0xf
	lea di, __plo_digitsh
	add ax, di
	mov di, ax
	xor ax, ax
	mov al, [di]
	or ax, #0x0a00
	seg es
	mov [bx], ax
	pop ax
	add bx, #2
	dec cx
	jnz __plo_debug_l0
	mov __plo_pos, bx
	pop di
	pop cx
	pop bx
	pop ax
	pop es
	ret

; MBR magic value
.org 0x1fe
	db 0x55, 0xaa

end
