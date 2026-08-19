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
#define XSPI2_STORAGE   1

#define VDDIO3_RANGE_MV 1800 /* XSPI port 2 - Flash */

// WIP Temporarily commented out to save some space in multidriver
// #define I2C1 1
//
// #define SPI5 1

#define TTY1                1
#define UART_CONSOLE_PLO    1
#define UART_CONSOLE_KERNEL 1

#define WIPDONTMERGE_NUCLEO 0

#if WIPDONTMERGE_NUCLEO
#define UART_CONSOLE_USER 1
#else
#define TTY7              1
#define UART_CONSOLE_USER 7
#endif

#if WIPDONTMERGE_NUCLEO
#define ETH1_MDC_PORT           gpiog
#define ETH1_MDC_PIN            11
#define ETH1_MDIO_PORT          gpiof
#define ETH1_MDIO_PIN           4
#define ETH1_RX_REFCLK_INTERNAL 0
#define ETH1_REF_CLK_PORT       gpiof
#define ETH1_REF_CLK_PIN        7
#define ETH1_MODE_RMII
#else
#define ETH1_MDC_PORT           gpioh
#define ETH1_MDC_PIN            5
#define ETH1_MDIO_PORT          gpiod
#define ETH1_MDIO_PIN           12
#define ETH1_PHY_INTN_PORT      gpiod
#define ETH1_PHY_INTN_PIN       3
#define ETH1_RX_REFCLK_INTERNAL 1
#define ETH1_CLK_OUTPUT         1
#define ETH1_MODE_RMII
#endif

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
