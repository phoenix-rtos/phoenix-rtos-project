/*
 * Phoenix-RTOS
 *
 * Board config for sparcv8leon3-gr712-rc
 *
 * Copyright 2023 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define SYSCLK_FREQ (48 * 1000 * 1000) /* MHz */

#define UART_MAX_CNT 6

#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0x80000100)
#define UART1_BASE ((void *)0x80100100)
#define UART2_BASE ((void *)0x80100200)
#define UART3_BASE ((void *)0x80100300)
#define UART4_BASE ((void *)0x80100400)
#define UART5_BASE ((void *)0x80100500)

#define UART0_IRQ 2
#define UART1_IRQ 17
#define UART2_IRQ 18
#define UART3_IRQ 19
#define UART4_IRQ 20
#define UART5_IRQ 21

#define UART0_TX 4
#define UART0_RX 3

#define UART1_TX 2
#define UART1_RX 1

#define UART2_TX 240
#define UART2_RX 239

#define UART3_TX 238
#define UART3_RX 233

#define UART4_TX 232
#define UART4_RX 231

#define UART5_TX 230
#define UART5_RX 229

#define UART0_ACTIVE 1
#define UART1_ACTIVE 1
#define UART2_ACTIVE 0
#define UART3_ACTIVE 0
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART_CONSOLE_PLO    0
#define UART_CONSOLE_KERNEL 0
#define UART_CONSOLE_USER   0

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

/* Interrupts */

#define INT_CTRL_BASE ((void *)0x80000200)
#define EXTENDED_IRQN 12

/* Timers */

#define TIMER0_CNT    4
#define TIMER1_CNT    2
#define TIMER_IRQ     8
#define GPTIMER0_BASE ((void *)0x80000300)
#define GPTIMER1_BASE ((void *)0x80100600)

/* SMP */

#define TLB_IRQ 14


#endif
