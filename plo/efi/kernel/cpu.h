/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * CPU related routines
 *
 * Copyright 2012, 2017 Phoenix Systems
 * Copyright 2001, 2006 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Michał Mirosław
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _HAL_CPU_H_
#define _HAL_CPU_H_

/* Bitfields used to construct interrupt descriptors */
#define IGBITS_DPL0       0x00000000
#define IGBITS_DPL3       0x00006000
#define IGBITS_PRES       0x00008000
#define IGBITS_SYSTEM     0x00000000
#define IGBITS_IRQEXC     0x00000e00
#define IGBITS_TRAP       0x00000f00
#define IGBITS_TSS        0x00000500


/* Bitfields used to construct segment descriptors */
#define DBITS_4KB         0x00800000    /* 4KB segment granularity */
#define DBITS_1B          0x00000000    /* 1B segment granularity */

#define DBITS_CODE32      0x00400000    /* 32-bit code segment */
#define DBITS_CODE16      0x00000000    /* 16-bit code segment */

#define DBITS_PRESENT     0x00008000    /* present segment */
#define DBITS_NOTPRESENT  0x00000000    /* segment not present in the physcial memory*/

#define DBITS_DPL0        0x00000000    /* kernel privilege level segment */
#define DBITS_DPL3        0x00006000    /* user privilege level segment */

#define DBITS_SYSTEM      0x00000000    /* segment used by system */
#define DBITS_APP         0x00001000    /* segment used by application */

#define DBITS_CODE        0x00000800    /* code segment descriptor */
#define DBITS_DATA        0x00000000    /* data segment descriptor */

#define DBITS_EXPDOWN     0x00000400    /* data segment is expandable down */
#define DBITS_WRT         0x00000200    /* writing to data segment is permitted */
#define DBITS_ACCESIBLE   0x00000100    /* data segment is accesible */

#define DBITS_CONFORM     0x00000400    /* conforming code segment */
#define DBITS_READ        0x00000200    /* read from code segment is permitted */


/*
 * Predefined descriptor types
 */


/* Descriptor of Task State Segment - used in CPU context switching */
#define DESCR_TSS    (DBITS_1B | DBITS_PRESENT | DBITS_DPL0 | DBITS_SYSTEM | 0x00000900)

/* Descriptor of user task code segment */
#define DESCR_UCODE  (DBITS_4KB | DBITS_CODE32 | DBITS_PRESENT | DBITS_DPL3 | DBITS_APP | DBITS_CODE | DBITS_READ)

/* Descriptor of user task data segment */
#define DESCR_UDATA  (DBITS_4KB | DBITS_CODE32 | DBITS_PRESENT | DBITS_DPL3 | DBITS_APP | DBITS_DATA | DBITS_WRT)


/* Descriptor of user task code segment */
#define DESCR_KCODE  (DBITS_4KB | DBITS_CODE32 | DBITS_PRESENT | DBITS_DPL0 | DBITS_APP | DBITS_CODE | DBITS_READ)

/* Descriptor of user task data segment */
#define DESCR_KDATA  (DBITS_4KB | DBITS_PRESENT | DBITS_DPL0 | DBITS_APP | DBITS_DATA | DBITS_WRT)


/* Segment selectors */
#define SEL_KCODE    8
#define SEL_KDATA    16
#define SEL_UCODE    27
#define SEL_UDATA    35


#define NULL ((void *)0)


#ifndef __ASSEMBLY__


#define PUTONSTACK(kstack, t, v) \
	(kstack) -= sizeof(t); \
	*((t *)kstack) = (v);


#define GETFROMSTACK(ustack, t, v, n) \
	do { \
		if (n == 0) \
			ustack += 4; \
		v = *(t *)ustack; \
		ustack += sizeof(t); \
	} while (0)


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef u32 addr_t;
typedef u64 cycles_t;

typedef u64 usec_t;
typedef s64 offs_t;

typedef unsigned int size_t;
typedef unsigned long long time_t;


#pragma pack(1)

/* CPU context saved by interrupt handlers on thread kernel stack */
typedef struct {
	u32 savesp;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 edx;
	u32 ecx;
	u32 ebx;
	u32 eax;
	u16 gs;
	u16 fs;
	u16 es;
	u16 ds;
	u32 eip; /* eip, cs, eflags, esp, ss saved by CPU on interrupt */
	u32 cs;
	u32 eflags;

	u32 esp;
	u32 ss;
} cpu_context_t;


/* IA32 TSS */
typedef struct {
	u16 backlink, _backlink;
	u32 esp0;
	u16 ss0, _ss0;
	u32 esp1;
	u16 ss1, _ss1;
	u32 esp2;
	u16 ss2, _ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u16 es, _es;
	u16 cs, _cs;
	u16 ss, _ss;
	u16 ds, _ds;
	u16 fs, _fs;
	u16 gs, _gs;
	u16 ldt, _ldt;
	u16 trfl;
	u16 iomap;
} tss_t;

#pragma pack(4)


/* io access */


static inline u8 hal_inb(void *addr)
{
	u32 ioport = (u32)addr;
	u8 b;

	__asm__ volatile
	(
		"inb %1, %0"
	: "=a" (b)
	: "dN" ((u16)ioport));

	return b;
}


static inline void hal_outb(void *addr, u8 b)
{
	u32 ioport = (u32)addr;

	__asm__ volatile
	(
		"outb %0, %1"
	:
	: "a" (b), "dN" ((u16)ioport));
}


static inline u16 hal_inw(void *addr)
{
	u32 ioport = (u32)addr;
	u16 w;

	__asm__ volatile
	(
		"inw %1, %0"
	: "=a" (w)
	: "dN" ((u16)ioport));

	return w;
}


static inline void hal_outw(void *addr, u16 w)
{
	u32 ioport = (u32)addr;

	__asm__ volatile
	(
		"outw %0, %1"
	:
	: "a" (w), "dN" ((u16)ioport));
}


static inline u32 hal_inl(void *addr)
{
	u32 ioport = (u32)addr;
	u32 l;

	__asm__ volatile
	(
		"inl %1, %0"
	: "=a" (l)
	: "dN" ((u16)ioport));

	return l;
}


static inline void hal_outl(void *addr, u32 l)
{
	u32 ioport = (u32)addr;

	__asm__ volatile
	(
		"outl %0, %1"
	:
	: "a" (l), "dN" ((u16)ioport));
}


static inline void hal_wrmsr(u32 id, u64 v)
{
	u32 l = (u32)v, h = v >> 32;
	__asm__ volatile
	(
		"wrmsr"
	:
	: "c" (id), "a" (l), "d" (h)
	: "memory");
}


static inline u64 hal_rdmsr(u32 id)
{
	u32 h, l;

	__asm__ volatile (
		"rdmsr"
	: "=a" (l), "=d" (h)
	: "c" (id)
	: "memory");

	return l | ((u64)h << 32);
}


/* interrupts */


static inline void hal_cpuDisableInterrupts(void)
{
	__asm__ volatile ("cli" ::: "memory");
}


static inline void hal_cpuEnableInterrupts(void)
{
	__asm__ volatile ("sti" ::: "memory");
}


/* performance */


static inline time_t hal_cpuLowPower(time_t ms)
{
	return 0;
}


static inline void hal_cpuHalt(void)
{
	__asm__ volatile ("hlt" ::: "memory");
}


static inline void hal_cpuGetCycles(void *cb)
{
	struct { u32 l, h; } *p = cb;

	__asm__ volatile
	(
		"rdtsc"
	: "=a" (p->l), "=d" (p->h)
	);
}


/* initial code helper */


__attribute__((pure))
static inline void *hal_piAddr(const void *addr)
{
	u32 reloffset;

	__asm__
	(" \
		call 1f; \
	1:	pop %0; \
		subl $1b, %0"
	: "=r" (reloffset));

	return (void *)addr + reloffset;
}


/* memory management */


static inline void hal_cpuFlushTLB(void *vaddr)
{
	unsigned long tmpreg;

	__asm__ volatile
	(" \
		movl %%cr3, %0; \
		movl %0, %%cr3"
	: "=r" (tmpreg)
	:
	: "memory");
}


static inline void *hal_cpuGetFaultAddr(void)
{
	u32 cr2;

	__asm__ volatile
	(
		"movl %%cr2, %0"
	: "=r" (cr2)
	:
	: "memory");

	return (void *)cr2;
}


static inline void hal_cpuSwitchSpace(addr_t cr3)
{
	__asm__ volatile
	(
		"movl %0, %%cr3"
	:
	: "r" (cr3)
	: "memory");
}


/* bit operations */


static inline unsigned int hal_cpuGetLastBit(u32 v)
{
	int lb = 0;

	if (v)
		__asm__
		(
			"bsrl %1, %0"
		: "=r" (lb)
		: "rm" (v));

	return lb;
}


static inline unsigned int hal_cpuGetFirstBit(u32 v)
{
	int fb = 0;

	if (v)
		__asm__
		(
			"bsfl %1, %0"
		: "=r" (fb)
		: "rm" (v));

	return fb;
}


/* debug */


static inline void hal_cpuSetBreakpoint(void *addr)
{
	__asm__ volatile
	(" \
		movl %0, %%dr3; \
		movl $0x80, %0; \
		movl %0, %%dr7"
	: "+r" (addr)
	:
	: "memory");
}


/* context management */


static inline void hal_cpuSetGot(cpu_context_t *ctx, void *got)
{
}


/* Function creates new cpu context on top of given thread kernel stack */
extern int hal_cpuCreateContext(cpu_context_t **nctx, void *start, void *kstack, size_t kstacksz, void *ustack, void *arg);


extern void hal_cpuReschedule(void);


static inline void hal_cpuRestore(cpu_context_t *curr, cpu_context_t *next)
{
	curr->savesp = (u32)next + sizeof(u32);
}


extern void _hal_cpuSetKernelStack(void *kstack);


extern void hal_cpuReturnFromSyscall(cpu_context_t *ctx, int val);


static inline void *hal_cpuGetSP(cpu_context_t *ctx)
{
	return (void *)ctx;
}


__attribute__((noreturn))
static inline void hal_longjmp(cpu_context_t *ctx)
{
	__asm__ volatile
	(" \
		cli; \
		movl %0, %%esp;\
		popl %%edi;\
		popl %%esi;\
		popl %%ebp;\
		popl %%edx;\
		popl %%ecx;\
		popl %%ebx;\
		popl %%eax;\
		popw %%gs;\
		popw %%fs;\
		popw %%es;\
		popw %%ds;\
		iret"
	:
	: "g" (&ctx->edi)
	: "memory");

	__builtin_unreachable();
}


__attribute__((noreturn))
static inline void hal_jmpSwitchStack(void *f, void *kstack, int argc)
{
	__asm__ volatile
	(" \
		movl %0, %%esp;\
		pushl $0;\
		jmpl *%1"
	:
	: "g" (kstack), "rm" (f)
	: "memory");

	__builtin_unreachable();
}


__attribute__((noreturn))
static inline void hal_enterUserMode(void *f, void *kstack, void *stack, int argc)
{
	__asm__ volatile
	(" \
		movl %0, %%esp;\
		sti; \
		pushl %4;\
		pushl %2;\
		pushfl;\
		pushl %3;\
		movw %w4, %%ds;\
		movw %w4, %%es;\
		movw %w4, %%fs;\
		movw %w4, %%gs;\
		pushl %1;\
		iret"
	:
	: "g" (kstack), "ri" (f), "r" (stack), "ri" ((u32)SEL_UCODE), "r" ((u32)SEL_UDATA)
	: "memory");

	__builtin_unreachable();
}


/* core management */


static inline unsigned int hal_cpuGetCount(void)
{
	return 1;
}


static inline unsigned int hal_cpuGetID(void)
{
	return 0;
}


extern void _hal_cpuInit(void);


#endif /* !__ASSEMBLY__ */


#endif
