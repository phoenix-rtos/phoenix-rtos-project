/*
 * Phoenix-RTOS
 *
 * Board config for armv7r5f-tda4vm-som
 *
 * Copyright 2025 Phoenix Systems
 * Author: Jacek Maksymowicz
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define WKUP_HFOSC0_HZ 19200000
#define WKUP_LFOSC0_HZ 32768
#define HFOSC1_HZ      22579200

#define UART0_BAUDRATE 115200
#define UART0_RX       pin_wkup_gpio0_13
#define UART0_TX       pin_wkup_gpio0_12

#define UART_CONSOLE_KERNEL 0
#define UART_CONSOLE_USER   0
#define UART_CONSOLE_PLO    0

#define UART16550_CONSOLE_USER UART_CONSOLE_USER
#define UART16550_BAUDRATE     UART0_BAUDRATE

#endif
