/*
 * Phoenix-RTOS
 *
 * Board config for sparcv8leon-gr740-mini
 *
 * Copyright 2025 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

/* CPU Configuration */

#define LEON_HAS_L2CACHE

#define SYSCLK_FREQ (250 * 1000 * 1000) /* MHz */

#define UART_MAX_CNT 2

#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0xff900000)
#define UART1_BASE ((void *)0xff901000)
#define UART2_BASE ((void *)-1)
#define UART3_BASE ((void *)-1)
#define UART4_BASE ((void *)-1)
#define UART5_BASE ((void *)-1)

#define UART0_IRQ 29
#define UART1_IRQ 30
#define UART2_IRQ -1
#define UART3_IRQ -1
#define UART4_IRQ -1
#define UART5_IRQ -1

#define UART0_TX 21
#define UART0_RX 9

#define UART1_TX 20
#define UART1_RX 8

#define UART2_TX -1
#define UART2_RX -1

#define UART3_TX -1
#define UART3_RX -1

#define UART4_TX -1
#define UART4_RX -1

#define UART5_TX -1
#define UART5_RX -1

#define UART0_ACTIVE 1
#define UART1_ACTIVE 0
#define UART2_ACTIVE 0
#define UART3_ACTIVE 0
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART0_DMA 0
#define UART1_DMA 0
#define UART2_DMA 0
#define UART3_DMA 0
#define UART4_DMA 0
#define UART5_DMA 0

#define UART_CONSOLE_PLO    0
#define UART_CONSOLE_KERNEL 0
#define UART_CONSOLE_USER   0

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

/* Interrupts */

#define INT_CTRL_BASE ((void *)0xff904000)
#define EXTENDED_IRQN 10

/* Timers */

#define TIMER0_1_IRQ  1
#define TIMER0_2_IRQ  2
#define GPTIMER0_BASE ((void *)0xff908000)
#define GPTIMER1_BASE ((void *)0xff909000)

/* ADC */

#define ADC_CNT 0

#define ADC0_ACTIVE 0
#define ADC1_ACTIVE 0
#define ADC2_ACTIVE 0
#define ADC3_ACTIVE 0
#define ADC4_ACTIVE 0
#define ADC5_ACTIVE 0
#define ADC6_ACTIVE 0
#define ADC7_ACTIVE 0

/* SPI */

#define SPI_CNT 1

/* GPIO */

#define GPIO_PORT_CNT 2

/* SpaceWire */

#define SPW_CNT 0

#define SPW0_ACTIVE 0
#define SPW1_ACTIVE 0
#define SPW2_ACTIVE 0
#define SPW3_ACTIVE 0
#define SPW4_ACTIVE 0
#define SPW5_ACTIVE 0

/* SMP */

#define TLB_IRQ 12

/* DMA */

#define DMA_MAX_CNT 0

/* SDRAM configuration */

#define SDCFG1_TRP  0 /* Effective 2 */
#define SDCFG1_TCAS 0 /* Effective 2 */
#define SDCFG1_TRFC 4 /* Effective 7 */

/* Configuration for IS42S1632F on GR740-MINI */
#define SDCFG1_BANKSZ 5
#define SDCFG1_COLSZ  2
#define SDCFG1_RFLOAD 780 /* For 100 MHz SDRAM clock */

#define SDCFG2_EN2T  1
#define SDCFG2_DCS   1
#define SDCFG2_BPARK 1

/* PROM configuration for S29GL01GT11DHV023 flash */

#define MCFG1_VAL 0x00000022 /* 32 R/W waitstates (128ns @ 250MHz) */
#define MCFG3_VAL 0x0
#define MCFG5_VAL 0x00000180 /* 3 lead out cycles (12ns @ 250MHz) */
#define MCFG7_VAL 0x0

#define ADDR_FLASH 0xc0000000

#define FTMCTRL_BASE 0xff903000

/* GRETH configuration */

#define GRETH_EDCL  0
#define GRETH_EUI48 0x862u

#endif
