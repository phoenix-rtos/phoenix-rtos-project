/*
 * Phoenix-RTOS
 *
 * Board config for sparcv8leon-gr716-mimas
 *
 * Copyright 2023 Phoenix Systems
 * Author: Lukasz Leczkowski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define SYSCLK_FREQ (100 * 1000 * 1000) /* MHz */

#define UART_MAX_CNT 2

#define UART_BAUDRATE 115200

#define UART0_BASE ((void *)0x80302000)
#define UART1_BASE ((void *)0x80303000)
#define UART2_BASE ((void *)-1)
#define UART3_BASE ((void *)-1)
#define UART4_BASE ((void *)-1)
#define UART5_BASE ((void *)-1)

#define UART0_IRQ 10
#define UART1_IRQ 12
#define UART2_IRQ -1
#define UART3_IRQ -1
#define UART4_IRQ -1
#define UART5_IRQ -1

#define UART0_TX 58
#define UART0_RX 59

#define UART1_TX 62
#define UART1_RX 61

#define UART2_TX -1
#define UART2_RX -1

#define UART3_TX -1
#define UART3_RX -1

#define UART4_TX -1
#define UART4_RX -1

#define UART5_TX -1
#define UART5_RX -1

#define UART0_ACTIVE 1
#define UART1_ACTIVE 1
#define UART2_ACTIVE 0
#define UART3_ACTIVE 0
#define UART4_ACTIVE 0
#define UART5_ACTIVE 0

#define UART0_DMA 1
#define UART1_DMA 0
#define UART2_DMA 0
#define UART3_DMA 0
#define UART4_DMA 0
#define UART5_DMA 0

#define UART_CONSOLE_PLO    1
#define UART_CONSOLE_KERNEL 1
#define UART_CONSOLE_USER   1

#define GPIO_DIR_IN  0
#define GPIO_DIR_OUT 1

#define PSEUDODEV 1

/* Interrupts */

#define TLB_IRQ       0
#define INT_CTRL_BASE ((void *)0x80002000)
#define EXTENDED_IRQN 1

/* Timers */

#define TIMER0_1_IRQ  9
#define TIMER0_2_IRQ  10
#define GPTIMER0_BASE ((void *)0x80003000)
#define GPTIMER1_BASE ((void *)0x80004000)

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
#define GRLIB_SPI_LOOPBACK

/* GPIO */

#define GPIO_PORT_CNT 0

/* SpaceWire */

#define SPW_CNT 0

#define SPW0_ACTIVE 0
#define SPW1_ACTIVE 0
#define SPW2_ACTIVE 0
#define SPW3_ACTIVE 0
#define SPW4_ACTIVE 0
#define SPW5_ACTIVE 0

/* DMA */

#define DMA_MAX_CNT 1


#endif
