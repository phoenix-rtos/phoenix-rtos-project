/*
 * Phoenix-RTOS
 *
 * Board config for riscv64-generic-spike
 *
 * Copyright 2022 Phoenix Systems
 * Author: Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

/* UART16550 configuration */
#define UART16550_CONSOLE_USER 0
#define UART16550_BAUDRATE     115200

/* SPIKE-TTY configuration */
#define SPIKETTY_CONSOLE_USER 0

/* Signals configuration */
#define SIGNALS_DISABLE 0

#endif
