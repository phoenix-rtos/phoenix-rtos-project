/*
 * Phoenix-RTOS
 *
 * Board config for riscv64-generic-qemu
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
#define UART16550_IRQ          10
#define UART16550_BASE         0x10000000

/* SPIKE-TTY configuration */
#define SPIKETTY_CONSOLE_USER 0

#define TIMER_FREQ (10 * 1000 * 1000)

#define RAM_ADDR      0x20000000u
#define RAM_BANK_SIZE 0x10000000u

#define PLIC_IRQ_SIZE 54

#endif
