/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Low-level routines
 *
 * Copyright 2012, 2020 Phoenix Systems
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "config.h"
#include "errors.h"
#include "low.h"
#include "plostd.h"


extern void _irq0(void);
extern void _irq1(void);
extern void _irq2(void);
extern void _irq3(void);
extern void _irq4(void);
extern void _irq5(void);
extern void _irq6(void);
extern void _irq7(void);
extern void _irq8(void);
extern void _irq9(void);
extern void _irq10(void);
extern void _irq11(void);
extern void _irq12(void);
extern void _irq13(void);
extern void _irq14(void);
extern void _irq15(void);


void *irqstubs[] = {
	_irq0, _irq1, _irq2, _irq3, _irq4, _irq5, _irq6, _irq7,
	_irq8, _irq9, _irq10, _irq11, _irq12, _irq13, _irq14, _irq15
};


struct {
	void *irqseg[16];
	void *irqoffs[16];
	void *data[16];
	void *isr[16];
} irqdata;


void low_cli(void)
{
#asm
	cli
#endasm
}


void low_sti()
{
#asm
	sti
#endasm
}


u8 low_inb(u16 addr)
{
#asm
	push bp
	mov bp, sp
	push dx

	mov dx, 4[bp]
	in al, dx

	pop dx
	pop bp
#endasm
}


void low_outb(u16 addr, u8 b)
{
#asm
	push bp
	mov bp, sp
	push dx

	mov dx, 4[bp]
	mov al, 6[bp]
	out dx, al

	pop dx
	pop bp
#endasm
}


u32 low_ind(u16 addr)
{
#asm
	push bp
	mov bp, sp

	mov dx, 4[bp]
	in eax, dx

	pop bp
	mov edx, eax
	shr edx, 16
#endasm
}


void low_outd(u16 addr, u32 d)
{
#asm
	push bp
	mov bp, sp
	push dx

	mov dx, 4[bp]
	mov eax, 6[bp]
	out dx, eax

	pop dx
	pop bp
#endasm
}


void low_setfar(u16 segm, u16 offs, u16 v)
{
#asm
	push bp
	mov bp, sp
	push bx
	push cx
	push dx
	push es

	mov dx, es
	mov ax, 4[bp]
	mov es, ax
	mov bx, 6[bp]
	mov cx, 8[bp]
	seg es
	mov word ptr [bx], cx
	mov es, dx

	pop es
	pop dx
	pop cx
	pop bx
	pop bp
#endasm
}


u16 low_getfar(u16 segm, u16 offs)
{
#asm
	push bp
	mov bp, sp
	push bx
	push dx

	mov dx, es
	mov ax, 4[bp]
	mov es, ax
	mov bx, 6[bp]
	seg es
	mov ax, word ptr [bx]
	mov es, dx

	pop dx
	pop bx
	pop bp
#endasm
}


void low_setfarbabs(u32 addr, u8 v)
{
#asm
	push bp
	mov bp, sp
	push ebx
	push fs

	xor ax, ax
	mov fs, ax
	mov eax, 4[bp]
	mov bl, 8[bp]
	seg fs
	mov byte ptr [eax], bl

	pop fs
	pop ebx
	pop bp
#endasm
}


u8 low_getfarbabs(u32 addr)
{
#asm
	push bp
	mov bp, sp
	push ebx
	push fs

	xor ax, ax
	mov fs, ax
	mov ebx, 4[bp]
	seg fs
	mov al, byte ptr [ebx]

	pop fs
	pop ebx
	pop bp
#endasm
}


void low_setfarabs(u32 addr, u32 v)
{
#asm
	push bp
	mov bp, sp
	push ebx
	push fs

	xor ax, ax
	mov fs, ax
	mov eax, 4[bp]
	mov ebx, 8[bp]
	seg fs
	mov dword ptr [eax], ebx

	pop fs
	pop ebx
	pop bp
#endasm
}


u32 low_getfarabs(u32 addr)
{
#asm
	push bp
	mov bp, sp
	push ebx
	push fs

	xor ax, ax
	mov fs, ax
	mov ebx, 4[bp]
	seg fs
	mov eax, dword ptr [ebx]

	pop fs
	pop ebx
	pop bp
#endasm
}


void low_copyto(u16 segm, u16 offs, void *src, unsigned int l)
{
#asm
	push bp
	mov bp, sp
	push cx
	push dx
	push si
	push di

	mov dx, es
	mov ax, 4[bp]
	mov es, ax
	mov cx, 10[bp]
	mov di, 6[bp]
	mov si, 8[bp]
	rep
	movsb
	mov es, dx

	pop di
	pop si
	pop dx
	pop cx
	pop bp
#endasm
}


void low_copyfrom(u16 segm, u16 offs, void *dst, unsigned int l)
{
#asm
	push bp
	mov bp, sp
	push cx
	push dx
	push si
	push di

	mov dx, ds
	mov ax, 4[bp]
	mov ds, ax
	mov cx, 10[bp]
	mov si, 6[bp]
	mov di, 8[bp]
	rep
	movsb
	mov ds, dx

	pop di
	pop si
	pop dx
	pop cx
	pop bp
#endasm
}


void low_memcpy(char *dst, char *src, unsigned int l)
{
#asm
	push bp
	mov bp, sp
	push cx
	push si
	push di

	mov cx, 8[bp]
	mov di, 4[bp]
	mov si, 6[bp]
	rep
	movsb

	pop di
	pop si
	pop cx
	pop bp
#endasm
}


void low_copytoabs(u32 addr, void *src, unsigned int l)
{
	unsigned int i;

	for (i = 0; i < l / sizeof(u32); i++)
		low_setfarabs(addr + i * sizeof(u32), *((u32 *)src + i));
	
	for (i *= sizeof(u32); i < l; i++)
		low_setfarbabs(addr + i, *((u8 *)src + i));
}


void low_copyfromabs(u32 addr, void *dst, unsigned int l)
{
	unsigned int i;

	for (i = 0; i < l / sizeof(u32); i++)
		*((u32 *)dst + i) = low_getfarabs(addr + i * sizeof(u32));
	
	for (i *= sizeof(u32); i < l; i++)
		*((u8 *)dst + i) = low_getfarbabs(addr + i);
}


u16 low_getcs(void)
{
#asm
	mov ax, cs
#endasm
}


void low_setmode(u8 m)
{
#asm
	push bp
	mov bp, sp
	mov ah, #0
	mov al, 4[bp]
	int 0x10
	pop bp
#endasm
}


#ifndef CONSOLE_SERIAL
void low_putc(char attr, char c)
{
#asm
	push bp
	mov bp, sp
	push bx
	push cx

	cmp 6[bp], #0x0a
	jnz low_putc_l1

; Perform carriage return
	xor bx, bx
	mov bl, 4[bp]
	mov al, #0x0d
	mov ah, #0x0e
	mov cx, #1
	int 0x10

low_putc_l1:
	xor bx, bx
	mov bl, 4[bp]
	mov al, 6[bp]
	mov ah, #0x0e
	mov cx, #1
	int 0x10

	pop cx
	pop bx
	pop bp
#endasm
}


void low_getc(char *c, char *sc)
{
#asm
	push bp
	mov bp, sp
	push bx

	mov ax, #0
	int 0x16
	mov bx, 4[bp]
	mov [bx], al
	mov bx, 6[bp]
	mov [bx], ah

	pop bx
	pop bp
#endasm
}
#endif


int low_keypressed(void)
{
#asm
	mov ax, #0x0100
	int 0x16
	jnz low_keypressed_l1
	xor ax, ax
	jmp low_keypressed_l2
low_keypressed_l1:
	mov ax, #1;
low_keypressed_l2:
#endasm
}


int low_int13read(u8 drive, u16 c, u8 h, u8 s, u8 n, u8 *buff)
{
#asm
	push bp
	mov bp, sp

	push bx
	push dx
	push cx

; read sector
	mov ah, #0x02

	mov cx, 6[bp]
	xchg ch, cl
	ror cl, #2     /* C */

	xor dx, dx
	mov dl, 10[bp]
	and dl, #0x3f
	or cx, dx      /* S */

	mov dh, 8[bp]  /* H */
	mov dl, 4[bp]  /* drive */
	mov al, 12[bp] /* n */
	mov bx, 14[bp] /* buffer */
	int 0x13
	jc low_int13read_l1
	mov ax, #0
low_int13read_l1:
	pop cx
	pop dx
	pop bx
	pop bp
#endasm
}


int low_int13param(u8 drive, u16 *mc, u8 *mh, u8 *ms)
{
#asm
	push bp
	mov bp, sp

	push bx
	push dx
	push cx
	push es

; get disk parameters
	mov ah, #0x08
	mov dl, 4[bp] /* drive */
	int 0x13
	jc low_int13param_l0

	mov bx, 6[bp]
	mov [bx], ch
	mov dl, cl
	and dl, #0xc0
	rol dl, #2
	mov 1[bx], dl  /* C */

	mov bx, 8[bp]
	mov [bx], dh   /* H */

	mov bx, 10[bp]
	and cl, #0x3f
	mov [bx], cl   /* S */

	mov ax, #0
	jmp low_int13param_l1

low_int13param_l0:
	/*mov ax, #1 */

low_int13param_l1:
	pop es
	pop cx
	pop dx
	pop bx
	pop bp
#endasm
}


/* Interrupt handling */


int low_irqdispatch(u16 irq)
{
	int (*isr)(u16, void *);

	if (irqdata.isr[irq] == NULL)
		return IRQ_DEFAULT;

	isr = irqdata.isr[irq];
	return isr(irq, irqdata.data[irq]);
}


void low_maskirq(u16 n, u8 v)
{
	u8 m, s;

	m = ~(1 << (n & 7));
	s = low_inb((n < 8) ? 0x21 : 0xa1) & m | ((v & 1) << (n & 7));

	low_outb((n < 8) ? 0x21 : 0xa1, s);
	return;
}


int low_irqinst(u16 irq, int (*isr)(u16, void *), void *data)
{
	if (irq > 15)
		return ERR_ARG;

	low_cli();
	irqdata.isr[irq] = (void *)isr;
	irqdata.data[irq] = data;
	low_sti();
	return 0;
}


int low_irquninst(u16 irq)
{
	if (irq > 15)
		return ERR_ARG;

	low_cli();
	irqdata.isr[irq] = NULL;
	irqdata.data[irq] = NULL;
	low_sti();
	return 0;
}


void low_irqinit(void)
{
	unsigned int k;

	/* Install IRQ stubs */
	low_cli();
	for (k = 0; k < 16; k++) {
		irqdata.irqoffs[k] = (void *)low_getfar(0, (k + (k < 8 ? 8 : 0x68)) * 4);
		irqdata.irqseg[k] = (void *)low_getfar(0, (k + (k < 8 ? 8 : 0x68)) * 4 + 2);

		low_setfar(0, (k + (k < 8 ? 8 : 0x68)) * 4, (u16)irqstubs[k]);
		low_setfar(0,  (k + (k < 8 ? 8 : 0x68)) * 4 + 2, low_getcs());

		irqdata.isr[k] = NULL;
		irqdata.data[k] = NULL;
	}
	low_sti();
	return;
}


void low_irqdone(void)
{
	unsigned int k;

	low_cli();

	/* Restore interrupt vector table */
	for (k = 0; k < 8; k++) {
		low_setfar(0, (k + (k < 8 ? 8 : 0x68)) * 4, (u16)irqdata.irqoffs[k]);
		low_setfar(0, (k + (k < 8 ? 8 : 0x68)) * 4 + 2, (u16)irqdata.irqseg[k]);
	}

	low_sti();
	return;
}


/* Kernel specific functions */


/* Function starts kernel loaded into memory */
int low_launch(void)
{
	low_cli();

	/* Prepare ring 0 code segment descriptor - selector 0x08 */
	low_setfar(GDT_SEG, 8, 0xffff);
	low_setfar(GDT_SEG, 10, 0x0000);
	low_setfar(GDT_SEG, 12, 0x9a00);
	low_setfar(GDT_SEG, 14, 0x00cf);

	/* Prepare ring 0 data segment descriptor - selector 0x10 */
	low_setfar(GDT_SEG, 16, 0xffff);
	low_setfar(GDT_SEG, 18, 0x0000);
	low_setfar(GDT_SEG, 20, 0x9200);
	low_setfar(GDT_SEG, 22, 0x00cf);

	/* Prepare GDTR pseudodescriptor */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 0, GDT_SIZE - 1);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 2, GDT_SEG << 4);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 4, GDT_SEG >> 12);

	/* Prepare IDTR pseudodescriptor */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_IDTR + 0, IDT_SIZE - 1);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_IDTR + 2, IDT_SEG << 4);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_IDTR + 4, IDT_SEG >> 12);

	/* Prepare paging parameters */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_PDIR + 0, PDIR_SEG << 4);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_PDIR + 2, PDIR_SEG >> 12);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_PTABLE + 0, PTABLE_SEG << 4);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_PTABLE + 2, PTABLE_SEG >> 12);

	/* Prepare kernel parameters */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_STACK + 0, INIT_ESP);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_STACK + 2, 0);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_STACKSIZE + 0, STACK_SIZE);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_STACKSIZE + 2, 0);

	/* Set kernel args */
	low_copyto(SYSPAGE_SEG, SYSPAGE_OFFS_ARG, KERNEL_ARGS, plostd_strlen(KERNEL_ARGS) + 1);

	/* Set text mode */
	low_setmode(0x03);

	/* Load protected mode specific registers */
#asm
	mov ax, #SYSPAGE_SEG
	mov es, ax
	seg es
	lgdt 0

	mov ax, #0
	mov ss, ax
	mov esp, #INIT_ESP
	mov eax, #SYSPAGE_SEG
	shl eax, #4
	push eax

	/* mov eax, cr0 */
	db 0x0f, 0x20, 0xc0

	/* or eax, 1 */
	db 0x66, 0x83, 0xc8, 0x01

	/* mov cr0, eax */
	db 0x0f, 0x22, 0xc0

	/* jmp far 0008h:00110000 (MOD) */
	db 0x66, 0xea, 0x00, 0x00, 0x11, 0x00, 0x08, 0x00
#endasm

	return;
}


/* Function retrieves memory map item and returns id for next call */
int low_int15e820(u32 id, low_mmitem_t *mmitem, u32 *nid)
{
#asm
	push bp
	mov bp, sp
	push ebx
	push ecx
	push edx
	push di

	mov eax, #0xe820
	mov ebx, 4[bp]
	mov ecx, #20
	mov edx, #0x534d4150
	mov di, 8[bp]
	int 0x15

	jnc bios15e820_success
	mov ax, #ERR_LOW_BIOS;
	jmp bios15e820_ret

bios15e820_success:
	mov di, 10[bp]
	mov [di], ebx
	xor ax, ax

bios15e820_ret:
	pop di
	pop edx
	pop ecx
	pop ebx
	pop bp
#endasm
}


/* Function prepares memory map for kernel */
int low_mmcreate(void)
{
	low_mmitem_t mmitem;
	unsigned int k;
	u32 id;

	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_MMSIZE, 0);

	for (id = 0, k = 0; k < MM_MAXSZ; k++) {
		if (low_int15e820(id, &mmitem, &id) < 0)
			return ERR_LOW_BIOS;

		low_copyto(SYSPAGE_SEG, SYSPAGE_OFFS_MM + k * sizeof(low_mmitem_t), &mmitem, sizeof(low_mmitem_t));
		low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_MMSIZE, k + 1);
		if (!id)
			return ERR_NONE;
	}

	return ERR_LOW_MMSIZE;
}


/* Function returns selected memory map item */
int low_mmget(unsigned int n, low_mmitem_t *mmitem)
{
	unsigned int mmsize;

	mmsize = low_getfar(SYSPAGE_SEG, SYSPAGE_OFFS_MMSIZE);
	if (n >= mmsize)
		return ERR_LOW_MMSIZE;
	low_copyfrom(SYSPAGE_SEG, SYSPAGE_OFFS_MM + n * sizeof(low_mmitem_t), mmitem, sizeof(low_mmitem_t));

	return ERR_NONE;
}


/* Initialization */


void low_unreal(void)
{
	/* Prepare ring 0 code segment descriptor (16-bit) - selector 0x08 */
	low_setfar(GDT_SEG, 8, 0xffff);
	low_setfar(GDT_SEG, 10, 0x0000);
	low_setfar(GDT_SEG, 12, 0x9a00);
	low_setfar(GDT_SEG, 14, 0x0000);

	/* Prepare ring 0 data segment descriptor - selector 0x10 */
	low_setfar(GDT_SEG, 16, 0xffff);
	low_setfar(GDT_SEG, 18, 0x0000);
	low_setfar(GDT_SEG, 20, 0x9200);
	low_setfar(GDT_SEG, 22, 0x00cf);

	/* Prepare GDTR pseudodescriptor */
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 0, GDT_SIZE - 1);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 2, GDT_SEG << 4);
	low_setfar(SYSPAGE_SEG, SYSPAGE_OFFS_GDTR + 4, GDT_SEG >> 12);

	low_cli();

#asm
	; Save real mode segment registers
	push ds
	push es
	push fs
	push gs

	; Load GDT into GDTR
	mov ax, #SYSPAGE_SEG
	mov es, ax
	seg es
	lgdt 0

	; Switch to pmode by setting pmode bit
	mov eax, cr0
	inc eax
	mov cr0, eax

	; jmp far prot
	db 0x66, 0xea
	dd prot + 0x7c00
	dw 0x08

prot:
	; Reload the segment registers to activate the new segment limits
	mov ax, #0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Back to realmode by toggling bit again
	mov eax, cr0
	dec eax
	mov cr0, eax
	jmp 0x7c0:real

real:
	; Reload the segment registers to match the base address and the selector
	pop gs
	pop fs
	pop es
	pop ds
#endasm

	low_sti();
}


/* Wait for kbd status bits with timeout */
static u8 low_waitkbdstatus(u8 bit, u8 state)
{
	u16 i;

	for (i = 0; i < 0xffffu; i++)
		if (!(low_inb(0x64) & (1 << bit) ^ (state << bit)))
			return 1;

	return 0;
}


/* Wait for kbd input buffer to be empty, so we can write */
static u8 low_waitkbdwrite(void)
{
	return low_waitkbdstatus(1, 0);
}


u8 low_a20(void)
{
	low_cli();

	/* Enable A20 using keyboard controller */
	if (!low_waitkbdwrite()) {
		low_sti();
		return 0;
	}

	low_outb(0x64, 0xd1);

	if (!low_waitkbdwrite()) {
		low_sti();
		return 0;
	}

	low_outb(0x60, 0xdf);

	if (!low_waitkbdwrite()) {
		low_sti();
		return 0;
	}

	low_sti();

	return 1;
}


void low_init(void)
{
	int res;
	int k;

	/* Enter unreal mode */
	low_unreal();
	/* Enable A20 line */
	low_a20();
	/* From now on we should have access to 4GB of memory */
	/* through zeroed out segment registers ds, es, fs and gs */

	/* Set graphics mode */
	low_setmode(0x12);

	if ((res = low_mmcreate()) < 0) {
		plostd_printf(ATTR_ERROR, "Problem in memory map creation [res=%x]!\n", res);
		if (res == ERR_LOW_BIOS)
			plostd_printf(ATTR_ERROR, "BIOS doesn't support int 15h, ax=e820h!\n");
		else
			plostd_printf(ATTR_ERROR, "Memory map has to many items >%d!\n", MM_MAXSZ);
		plostd_printf(ATTR_ERROR, "Phoenix can work unstable!\n");
	}

	/* Initialize interrupts */
	low_irqinit();
	return;
}


void low_done(void)
{
	low_irqdone();
	return;
}
