/*
 * Phoenix-RTOS
 *
 * Board config for ia32-generic-pc
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

/* pc-tty configuration */
#define PC_TTY_CREATE_PS2_VDEVS  0
#define PC_TTY_MOUSE_ENABLE      0
#define PC_TTY_MOUSE_SKIP_CONFIG 1

#endif
