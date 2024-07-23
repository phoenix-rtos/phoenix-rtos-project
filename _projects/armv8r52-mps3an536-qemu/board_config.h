/*
 * Phoenix-RTOS
 *
 * Board config for armv8r52-mps3an536-qemu
 *
 * Copyright 2024 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_


/* Peripherals */

#define UART_MAX_CNT 6

#define UART_CONSOLE_PLO    2
#define UART_CONSOLE_KERNEL 2
#define UART_CONSOLE_USER   2

#define UART_BAUDRATE 115200

/* UART0 accessible only by CPU0 */
#define UART0_BASE ((void *)0xe7c00000)
/* CPU0 private interrupts */
#define UART0_RX_IRQ   16
#define UART0_TX_IRQ   17
#define UART0_COMB_IRQ 18
#define UART0_OVFL_IRQ 19
#define UART0_ACTIVE   0

/* UART1 accessible only by CPU1 */
#define UART1_BASE UART0_BASE
/* CPU1 private interrupt */
#define UART1_RX_IRQ   UART0_RX_IRQ
#define UART1_TX_IRQ   UART0_TX_IRQ
#define UART1_COMB_IRQ UART0_COMB_IRQ
#define UART1_OVFL_IRQ UART0_OVFL_IRQ
#define UART1_ACTIVE   0

/* UARTs accessible by both CPUs */
#define UART2_BASE     ((void *)0xe0205000)
#define UART2_RX_IRQ   (32 + 5)
#define UART2_TX_IRQ   (32 + 6)
#define UART2_COMB_IRQ (32 + 13)
#define UART2_ACTIVE   1

#define UART3_BASE     ((void *)0xe0206000)
#define UART3_RX_IRQ   (32 + 7)
#define UART3_TX_IRQ   (32 + 8)
#define UART3_COMB_IRQ (32 + 14)
#define UART3_ACTIVE   0

#define UART4_BASE     ((void *)0xe0207000)
#define UART4_RX_IRQ   (32 + 9)
#define UART4_TX_IRQ   (32 + 10)
#define UART4_COMB_IRQ (32 + 15)
#define UART4_ACTIVE   0

#define UART5_BASE     ((void *)0xe0208000)
#define UART5_RX_IRQ   (32 + 11)
#define UART5_TX_IRQ   (32 + 12)
#define UART5_COMB_IRQ (32 + 16)
#define UART5_ACTIVE   0


#define TIMER_BASE ((void *)0xe0101000)
#define TIMER_IRQ  (32 + 1)


#define SYSCLK_FREQ (50 * 1000 * 1000) /* 50 MHz */


#endif
