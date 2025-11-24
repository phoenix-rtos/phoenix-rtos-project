/*
 * Phoenix-RTOS
 *
 * Board config for sparcv8leon-gr716-mini
 *
 * Copyright 2022 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define SYSCLK_FREQ (50 * 1000 * 1000) /* MHz */

#define UART_MAX_CNT 6

#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0x80300000U)
#define UART1_BASE ((void *)0x80301000U)
#define UART2_BASE ((void *)0x80302000U)
#define UART3_BASE ((void *)0x80303000U)
#define UART4_BASE ((void *)0x80304000U)
#define UART5_BASE ((void *)0x80305000U)

#define UART0_IRQ 24
#define UART1_IRQ 25
#define UART2_IRQ 3
#define UART3_IRQ 5
#define UART4_IRQ 6
#define UART5_IRQ 7

#define UART0_TX 2
#define UART0_RX 3

#define UART1_TX 4
#define UART1_RX 5

#define UART2_TX 58
#define UART2_RX 59

#define UART3_TX 62
#define UART3_RX 61

#define UART4_TX 38
#define UART4_RX 39

#define UART5_TX 43
#define UART5_RX 44

#define UART0_ACTIVE 0
#define UART1_ACTIVE 0
#define UART2_ACTIVE 1
#define UART3_ACTIVE 1
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART0_DMA 0
#define UART1_DMA 0
#define UART2_DMA 0
#define UART3_DMA 0
#define UART4_DMA 0
#define UART5_DMA 0

#define UART_CONSOLE_PLO    2
#define UART_CONSOLE_KERNEL 2
#define UART_CONSOLE_USER   2

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

#define PSEUDODEV 1

/* Interrupts */

#define TLB_IRQ       0
#define INT_CTRL_BASE ((void *)0x80002000U)
#define EXTENDED_IRQN 1

/* Timers */

#define TIMER0_1_IRQ  9
#define TIMER0_2_IRQ  10
#define GPTIMER0_BASE ((void *)0x80003000U)
#define GPTIMER1_BASE ((void *)0x80004000U)


/* ADC */

#define ADC_CNT 8

#define ADC0_ACTIVE 1
#define ADC1_ACTIVE 0
#define ADC2_ACTIVE 0
#define ADC3_ACTIVE 0
#define ADC4_ACTIVE 0
#define ADC5_ACTIVE 0
#define ADC6_ACTIVE 0
#define ADC7_ACTIVE 0

/* SPI */

#define SPI_CNT 2

/* GPIO */

#define GPIO_PORT_CNT 2

/* SpaceWire */

#define SPW_CNT 1

#define SPW0_ACTIVE 0
#define SPW1_ACTIVE 0
#define SPW2_ACTIVE 0
#define SPW3_ACTIVE 0
#define SPW4_ACTIVE 0
#define SPW5_ACTIVE 0

/* DMA */

#define DMA_MAX_CNT 0


/* Flash */

#define FLASH_CNT 1

#define SPIMCTRL0_BASE ((void *)0xfff00100U)
#define FLASH0_ADDR    0x02000000U

#endif
