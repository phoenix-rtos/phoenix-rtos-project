/*
 * Phoenix-RTOS
 *
 * Board config for riscv64-noelv-fpga
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

/* GRLIB UART configuration */
#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0xfc001000)
#define UART1_BASE ((void *)0xfc0d0000)
#define UART2_BASE ((void *)-1)
#define UART3_BASE ((void *)-1)
#define UART4_BASE ((void *)-1)
#define UART5_BASE ((void *)-1)

#define UART0_IRQ 1
#define UART1_IRQ 9
#define UART2_IRQ -1
#define UART3_IRQ -1
#define UART4_IRQ -1
#define UART5_IRQ -1

#define UART0_TX -1
#define UART0_RX -1

#define UART1_TX -1
#define UART1_RX -1

#define UART2_TX -1
#define UART2_RX -1

#define UART3_TX -1
#define UART3_RX -1

#define UART4_TX -1
#define UART4_RX -1

#define UART5_TX -1
#define UART5_RX -1

#define UART0_ACTIVE 1
#define UART1_ACTIVE 1
#define UART2_ACTIVE 0
#define UART3_ACTIVE 0
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART_CONSOLE_USER     0
#define SPIKETTY_CONSOLE_USER 0

#define UART_MAX_CNT 2

#define TIMER_FREQ (20 * 1000 * 1000)

#define PLIC_BASE     0xf8000000UL
#define PLIC_IRQ_SIZE 32

#define SYSCLK_FREQ (40 * 1000 * 1000)

#define SBI_AREA_START 0x0UL
#define SBI_AREA_END   0x10000UL

#define DCACHE_BLOCK_SIZE 0x1000U


/* Flash */

#define FLASH_CNT 1

#define SPIMCTRL0_BASE ((void *)0xffff0000)
#define FLASH0_ADDR    0x90000000u


#endif
