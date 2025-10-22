/*
 * Phoenix-RTOS
 *
 * Board config for riscv64-gr765-fpga
 *
 * Copyright 2025 Phoenix Systems
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

#define UART0_BASE ((void *)0xff900000)
#define UART1_BASE ((void *)-1)
#define UART2_BASE ((void *)-1)
#define UART3_BASE ((void *)-1)
#define UART4_BASE ((void *)-1)
#define UART5_BASE ((void *)-1)

#define UART0_IRQ 1
#define UART1_IRQ -1
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
#define UART1_ACTIVE 0
#define UART2_ACTIVE 0
#define UART3_ACTIVE 0
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART_CONSOLE_PLO      0
#define UART_CONSOLE_KERNEL   0
#define UART_CONSOLE_USER     0
#define SPIKETTY_CONSOLE_USER 0

#define UART_MAX_CNT 1

#define TIMER_FREQ (50 * 1000 * 1000)

#define PLIC_BASE     0xf8000000UL
#define PLIC_IRQ_SIZE 32

#define SYSCLK_FREQ (100 * 1000 * 1000)

#define SBI_AREA_START 0x0UL
#define SBI_AREA_END   0x10000UL

#define DCACHE_BLOCK_SIZE 32U

/* RAM storage config */
#define RAM_ADDR      (0x10000000)
#define RAM_BANK_SIZE (0x10000000)

/* SpaceWire GRSPW2 */

#define SPW_CNT 2

#define SPW0_ACTIVE 1
#define SPW1_ACTIVE 1
#define SPW2_ACTIVE 0
#define SPW3_ACTIVE 0
#define SPW4_ACTIVE 0
#define SPW5_ACTIVE 0

#define SPW_MAX_PACKET_LEN 1024

#define TEST_SPW_ADDR0 0x3
#define TEST_SPW_ADDR1 0x4

/* if ports are connected via loopback */
#define TEST_SPW_LOOPBACK 0

/* SpaceWire SPWRTR */

#define SPWRTR_CNT 1

#define SPWRTR0_ACTIVE 1
#define SPWRTR1_ACTIVE 0
#define SPWRTR2_ACTIVE 0
#define SPWRTR3_ACTIVE 0
#define SPWRTR4_ACTIVE 0
#define SPWRTR5_ACTIVE 0

#endif
