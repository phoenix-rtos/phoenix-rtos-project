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


/*
 * libpseudodev and libposixsrv shall be used exclusively, libpseudodev uses
 * less resources, but libposixsrv provides POSIX support and may be resource
 * hungry, by default libposixsrv is enabled.
 */

/* #define PSEUDODEV 1 */
#define BUILTIN_POSIXSRV 1

/* plo params */

#define RAM_ADDR      0x20000000
#define RAM_BANK_SIZE (384 * 1024)
#define UART_MAX_CNT  10


/* Peripherals */

/* FlexComm configuration options */
#define FLEXCOMM_NONE    0
#define FLEXCOMM_UART    1
#define FLEXCOMM_SPI     2
#define FLEXCOMM_I2C     3
#define FLEXCOMM_UARTI2C 7

/* FlexComm selections */
#define FLEXCOMM0_SEL FLEXCOMM_NONE
#define FLEXCOMM1_SEL FLEXCOMM_NONE
#define FLEXCOMM2_SEL FLEXCOMM_NONE
#define FLEXCOMM3_SEL FLEXCOMM_NONE
#define FLEXCOMM4_SEL FLEXCOMM_UART
#define FLEXCOMM5_SEL FLEXCOMM_NONE
#define FLEXCOMM6_SEL FLEXCOMM_NONE
#define FLEXCOMM7_SEL FLEXCOMM_NONE
#define FLEXCOMM8_SEL FLEXCOMM_NONE
#define FLEXCOMM9_SEL FLEXCOMM_NONE

#define UART_CONSOLE 4

/* MCXTODO */

#endif
