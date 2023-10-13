/*
 * Phoenix-RTOS
 *
 * Board config for sparcv8leon3-gr716-mini
 *
 * Copyright 2022 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define SYSCLK_FREQ (50 * 1000 * 1000) /* MHz */

#define UART_MAX_CNT 6

#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0x80300000)
#define UART1_BASE ((void *)0x80301000)
#define UART2_BASE ((void *)0x80302000)
#define UART3_BASE ((void *)0x80303000)
#define UART4_BASE ((void *)0x80304000)
#define UART5_BASE ((void *)0x80305000)

#define UART0_IRQ 24
#define UART1_IRQ 25
#define UART2_IRQ 3
#define UART3_IRQ 5
#define UART4_IRQ 6
#define UART5_IRQ 7

#define UART0_TX 2
#define UART0_RX 3

#define UART1_TX 4
#define UART1_RX 5

#define UART2_TX 58
#define UART2_RX 59

#define UART3_TX 62
#define UART3_RX 61

#define UART4_TX 38
#define UART4_RX 39

#define UART5_TX 43
#define UART5_RX 44

#define UART0_ACTIVE 0
#define UART1_ACTIVE 0
#define UART2_ACTIVE 1
#define UART3_ACTIVE 1
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART_CONSOLE_PLO    2
#define UART_CONSOLE_KERNEL 2
#define UART_CONSOLE_USER   2

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

#define PSEUDODEV 1
#define MULTI_SPI
#define MULTI_GPIO
#define MULTI_ADC

#endif
