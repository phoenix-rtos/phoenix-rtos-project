/*
 * Phoenix-RTOS
 *
 * Board config for armv8m33-nrf9160-devkit
 *
 * Copyright 2022 Phoenix Systems
 * Author: Lukasz Kosinski, Damian Loewnau
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#define TTY0         1
#define UART_CONSOLE 0
#define UART_CONSOLE_PLO 0

/* currently supported baud rates: 9600, 115200 */
#define UART_BAUDRATE 115200

/* sizes of uart dma memory regions - 1 because of max value of maxcnt */
#define UART_TX_DMA_SIZE 8191
#define UART_RX_DMA_SIZE 8191

/* the configuration of uart pins can be changed - every pin can be used as uart alternative function */

/* default uart instance for nrf9160 dk, connected to VCOM0 */
#define UART0_TX 29
#define UART0_RX 28
#define UART0_RTS 27
#define UART0_CTS 26

/* second uart interface on nrf9160 dk called nRF91_UART_2 on the board's schematic*/
#define UART1_TX 1
#define UART1_RX 0
#define UART1_RTS 14
#define UART1_CTS 15

#define UART2_TX UART0_TX
#define UART2_RX UART0_RX
#define UART2_RTS UART0_RTS
#define UART2_CTS UART0_CTS

#define UART3_TX UART1_TX
#define UART3_RX UART1_RX
#define UART3_RTS UART1_RTS
#define UART3_CTS UART1_CTS

/* ram7: section 2 and 3 */
#define UART0_TX_DMA 0x2003C000
#define UART0_RX_DMA 0x2003E000

/* ram7: section 0 and 1 */
#define UART1_TX_DMA 0x20038000
#define UART1_RX_DMA 0x2003A000

#define UART2_TX_DMA UART0_TX_DMA
#define UART2_RX_DMA UART0_RX_DMA

#define UART3_TX_DMA UART1_TX_DMA
#define UART3_RX_DMA UART1_RX_DMA

#endif
