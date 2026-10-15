/*
 * Phoenix-RTOS
 *
 * Board config for armv8m33-stm32u3-nucleo_u3c5
 *
 * Copyright 2026 Apator Metrix
 * Author: Mateusz Karcz
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define UART_IO_PORT gpioa
#define UART_IO_AF   7
#define UART_PIN_TX  9
#define UART_PIN_RX  10

#define TTY1                1
#define UART_CONSOLE_PLO    1
#define UART_CONSOLE_KERNEL 1
#define UART_CONSOLE_USER   1

#endif
