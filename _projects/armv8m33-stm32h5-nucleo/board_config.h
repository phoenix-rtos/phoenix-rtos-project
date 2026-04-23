/*
 * Phoenix-RTOS
 *
 * Board config for armv8m33-stm32h5-nucleo
 *
 * Copyright 2024, 2025, 2026 Phoenix Systems
 * Author: Aleksander Kaminski, Jacek Maksymowicz
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_


#define UART_IO_PORT gpiob
#define UART_IO_AF   14
#define UART_PIN_TX  13
#define UART_PIN_RX  12

#define TTY5                1
#define UART_CONSOLE_PLO    5
#define UART_CONSOLE_KERNEL 5
#define UART_CONSOLE_USER   5

/*
 * libpseudodev and libposixsrv shall be used exclusively, libpseudodev uses
 * less resources, but libposixsrv provides POSIX support and may be resource
 * hungry, by default libposixsrv is enabled.
 */

/* #define PSEUDODEV 1 */
#define BUILTIN_POSIXSRV 1
#define BUILTIN_DUMMYFS  1

/* plo params */

#endif
