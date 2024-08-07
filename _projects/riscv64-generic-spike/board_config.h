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
#define UART16550_IRQ          1
#define UART16550_BASE         0x10000000

/* SPIKE-TTY configuration */
#define SPIKETTY_CONSOLE_USER 0

#define TIMER_FREQ (10 * 1000 * 1000)

#define RAM_ADDR      0x80200000u
#define RAM_BANK_SIZE 0x10000000u

#define PLIC_BASE     0xc000000
#define PLIC_IRQ_SIZE 32

#define SBI_AREA_START 0x80000000UL
#define SBI_AREA_END   0x80200000UL

#endif
