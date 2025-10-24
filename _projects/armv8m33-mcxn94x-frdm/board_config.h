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

// #define PSEUDODEV 1
#define BUILTIN_POSIXSRV 1
#define BUILTIN_DUMMYFS  1

/* plo params */

#define RAM_ADDR      0x20000000
#define RAM_BANK_SIZE (416 * 1024)
#define UART_MAX_CNT  10


/* Peripherals */

/* ROSC */
#define ROSC_EXTALCAP_PF 12 /* 0-30 pF */
#define ROSC_CAP_PF      12 /* 0-30 pF */
#define ROSC_AMP_GAIN    0  /* 0-3 */

/* SOSC */
#define SOSC_FREQ (24 * 1000 * 1000)

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

#define FLEXCOMM0_BASE ((void *)0x40092000)
#define FLEXCOMM1_BASE ((void *)0x40093000)
#define FLEXCOMM2_BASE ((void *)0x40094000)
#define FLEXCOMM3_BASE ((void *)0x40095000)
#define FLEXCOMM4_BASE ((void *)0x400b4000)
#define FLEXCOMM5_BASE ((void *)0x400b5000)
#define FLEXCOMM6_BASE ((void *)0x400b6000)
#define FLEXCOMM7_BASE ((void *)0x400b7000)
#define FLEXCOMM8_BASE ((void *)0x400b8000)
#define FLEXCOMM9_BASE ((void *)0x400b9000)

#define FLEXCOMM0_IRQ lp_flexcomm0_irq
#define FLEXCOMM1_IRQ lp_flexcomm1_irq
#define FLEXCOMM2_IRQ lp_flexcomm2_irq
#define FLEXCOMM3_IRQ lp_flexcomm3_irq
#define FLEXCOMM4_IRQ lp_flexcomm4_irq
#define FLEXCOMM5_IRQ lp_flexcomm5_irq
#define FLEXCOMM6_IRQ lp_flexcomm6_irq
#define FLEXCOMM7_IRQ lp_flexcomm7_irq
#define FLEXCOMM8_IRQ lp_flexcomm8_irq
#define FLEXCOMM9_IRQ lp_flexcomm9_irq

#define UART_CONSOLE 4
#define UART_CLK     12000000 /* FRO_12M */

#define UART0_BAUDRATE 115200
#define UART0_BUFFSZ   512
#define UART0_RX_PIN   -1
#define UART0_TX_PIN   -1
#define UART0_RX_ALT   -1
#define UART0_TX_ALT   -1

#define UART1_BAUDRATE 115200
#define UART1_BUFFSZ   512
#define UART1_RX_PIN   -1
#define UART1_TX_PIN   -1
#define UART1_RX_ALT   -1
#define UART1_TX_ALT   -1

#define UART2_BAUDRATE 115200
#define UART2_BUFFSZ   512
#define UART2_RX_PIN   -1
#define UART2_TX_PIN   -1
#define UART2_RX_ALT   -1
#define UART2_TX_ALT   -1

#define UART3_BAUDRATE 115200
#define UART3_BUFFSZ   512
#define UART3_RX_PIN   -1
#define UART3_TX_PIN   -1
#define UART3_RX_ALT   -1
#define UART3_TX_ALT   -1

#define UART4_BAUDRATE 115200
#define UART4_BUFFSZ   512
#define UART4_RX_PIN   pctl_pin_p1_8
#define UART4_TX_PIN   pctl_pin_p1_9
#define UART4_RX_ALT   2
#define UART4_TX_ALT   2

#define UART5_BAUDRATE 115200
#define UART5_BUFFSZ   512
#define UART5_RX_PIN   -1
#define UART5_TX_PIN   -1
#define UART5_RX_ALT   -1
#define UART5_TX_ALT   -1

#define UART6_BAUDRATE 115200
#define UART6_BUFFSZ   512
#define UART6_RX_PIN   -1
#define UART6_TX_PIN   -1
#define UART6_RX_ALT   -1
#define UART6_TX_ALT   -1

#define UART7_BAUDRATE 115200
#define UART7_BUFFSZ   512
#define UART7_RX_PIN   -1
#define UART7_TX_PIN   -1
#define UART7_RX_ALT   -1
#define UART7_TX_ALT   -1

#define UART8_BAUDRATE 115200
#define UART8_BUFFSZ   512
#define UART8_RX_PIN   -1
#define UART8_TX_PIN   -1
#define UART8_RX_ALT   -1
#define UART8_TX_ALT   -1

#define UART9_BAUDRATE 115200
#define UART9_BUFFSZ   512
#define UART9_RX_PIN   -1
#define UART9_TX_PIN   -1
#define UART9_RX_ALT   -1
#define UART9_TX_ALT   -1

#endif
