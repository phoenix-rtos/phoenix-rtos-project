/*
 * Phoenix-RTOS
 *
 * Board config for aarch64a53-zynqmp-zcu104
 *
 * Copyright 2022, 2024 Phoenix Systems
 * Author: Lukasz Kosinski, Jacek Maksymowicz
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

/* SPI configuration */
#define SPI0_SCLK -1
#define SPI0_MOSI -1
#define SPI0_MISO -1
#define SPI0_SS0  -1
#define SPI0_SS1  -1
#define SPI0_SS2  -1

#define SPI1_SCLK -1
#define SPI1_MOSI -1
#define SPI1_MISO -1
#define SPI1_SS0  -1
#define SPI1_SS1  -1
#define SPI1_SS2  -1

/* I2C configuration */
#define I2C0_SDA -1
#define I2C0_SCL -1

#define I2C1_SDA 17
#define I2C1_SCL 16

/* UART configuration */
#define UART0_RX 18
#define UART0_TX 19
#define UART1_RX 21
#define UART1_TX 20

#define UART_CONSOLE_KERNEL 1
#define UART_CONSOLE_USER   1
#define UART_CONSOLE_PLO    1

/* CAN bus configuration */
#define CAN0_RX 25
#define CAN0_TX 24

/* UART16550 configuration */
#define UART16550_CONSOLE_USER -1
#define UART16550_BAUDRATE      115200

/* QSPI configuration */
#define QSPI_CLK  0
#define QSPI_IO1  1
#define QSPI_IO2  2
#define QSPI_IO3  3
#define QSPI_IO0  4
#define QSPI_CS   5
#define QSPI_FCLK 6

/* SD card configuration */
#define SD_CARD_CLK 51
#define SD_CARD_CMD 50
#define SD_CARD_D0  46
#define SD_CARD_D1  47
#define SD_CARD_D2  48
#define SD_CARD_D3  49
#define SD_CARD_CD  45

/* GPIO bank 0 configuration */
#define GPIO0_0  -1
#define GPIO0_1  -1
#define GPIO0_2  -1
#define GPIO0_3  -1
#define GPIO0_4  -1
#define GPIO0_5  -1
#define GPIO0_6  -1
#define GPIO0_7  -1
#define GPIO0_8  -1
#define GPIO0_9  -1
#define GPIO0_10 -1
#define GPIO0_11 -1
#define GPIO0_12 -1
#define GPIO0_13 -1
#define GPIO0_14 -1
#define GPIO0_15 -1
#define GPIO0_16 -1
#define GPIO0_17 -1
#define GPIO0_18 -1
#define GPIO0_19 -1
#define GPIO0_20 -1
#define GPIO0_21 -1
#define GPIO0_22 -1
#define GPIO0_23 -1
#define GPIO0_24 -1
#define GPIO0_25 -1
#define GPIO0_26 -1
#define GPIO0_27 -1
#define GPIO0_28 -1
#define GPIO0_29 -1
#define GPIO0_30 -1
#define GPIO0_31 -1

/* GPIO bank 1 configuration */
#define GPIO1_0  -1
#define GPIO1_1  -1
#define GPIO1_2  -1
#define GPIO1_3  -1
#define GPIO1_4  -1
#define GPIO1_5  -1
#define GPIO1_6  -1
#define GPIO1_7  -1
#define GPIO1_8  -1
#define GPIO1_9  -1
#define GPIO1_10 -1
#define GPIO1_11 -1
#define GPIO1_12 -1
#define GPIO1_13 -1
#define GPIO1_14 -1
#define GPIO1_15 -1
#define GPIO1_16 -1
#define GPIO1_17 -1
#define GPIO1_18 -1
#define GPIO1_19 -1
#define GPIO1_20 -1
#define GPIO1_21 -1

#endif
