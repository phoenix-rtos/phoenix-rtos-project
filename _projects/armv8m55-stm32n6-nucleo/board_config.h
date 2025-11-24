/*
 * Phoenix-RTOS
 *
 * Board config for armv8m55-stm32n6-nucleo
 *
 * Copyright 2024, 2025 Phoenix Systems
 * Author: Aleksander Kaminski, Jacek Maksymowicz
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

#define XSPI2           1
#define XSPI2_CLOCK_DIV 1 /* 200 MHz */

#define VDDIO3_RANGE_MV 1800 /* XSPI port 2 - Flash */

#define I2C1 1

#define SPI5 1

#define TTY1                1
#define UART_CONSOLE_PLO    1
#define UART_CONSOLE_KERNEL 1
#define UART_CONSOLE_USER   1

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
