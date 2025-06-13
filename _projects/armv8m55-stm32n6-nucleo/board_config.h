/*
 * Phoenix-RTOS
 *
 * Board config for armv8m33-mcxn94x-frdm
 *
 * Copyright 2024 Phoenix Systems
 * Author: Aleksander Kaminski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_


#define UART_IO_PORT gpioe
#define UART_IO_AF   7
#define UART_PIN_TX  5
#define UART_PIN_RX  6

#define TTY1                1
#define UART_CONSOLE_PLO    1
#define UART_CONSOLE_KERNEL 1
#define UART_CONSOLE_USER   1

/*
 * libpseudodev and libposixsrv shall be used exclusively, libpseudodev uses
 * less resources, but libposixsrv provides POSIX support and may be resource
 * hungry, by default libposixsrv is enabled.
 */

// #define PSEUDODEV 1
#define BUILTIN_POSIXSRV 1
#define BUILTIN_DUMMYFS  1

/* plo params */

#endif
