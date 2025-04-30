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

#define OSPI0_CLK    pin_mcu_ospi0_clk
#define OSPI0_LBCLKO pin_mcu_ospi0_lbclko
#define OSPI0_DQS    pin_mcu_ospi0_dqs
#define OSPI0_D0     pin_mcu_ospi0_d0
#define OSPI0_D1     pin_mcu_ospi0_d1
#define OSPI0_D2     pin_mcu_ospi0_d2
#define OSPI0_D3     pin_mcu_ospi0_d3
#define OSPI0_D4     pin_mcu_ospi0_d4
#define OSPI0_D5     pin_mcu_ospi0_d5
#define OSPI0_D6     pin_mcu_ospi0_d6
#define OSPI0_D7     pin_mcu_ospi0_d7
#define OSPI0_CS     pin_mcu_ospi0_csn0

#define OSPI0_PIN_MUX 0

#define OSPI1_CLK    pin_mcu_ospi1_clk
#define OSPI1_LBCLKO pin_mcu_ospi1_lbclko
#define OSPI1_DQS    pin_mcu_ospi1_dqs
#define OSPI1_D0     pin_mcu_ospi1_d0
#define OSPI1_D1     pin_mcu_ospi1_d1
#define OSPI1_D2     pin_mcu_ospi1_d2
#define OSPI1_D3     pin_mcu_ospi1_d3
#define OSPI1_CS     pin_mcu_ospi1_csn0

#define OSPI1_PIN_MUX 0

#define UART_CONSOLE_KERNEL 0
#define UART_CONSOLE_USER   0
#define UART_CONSOLE_PLO    0

#define UART16550_CONSOLE_USER UART_CONSOLE_USER
#define UART16550_BAUDRATE     UART0_BAUDRATE

#endif
