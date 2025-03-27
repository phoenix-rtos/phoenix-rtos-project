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

/* DDR controller parameters */
#define DDRC_DERATEEN_VAL   0x00000200
#define DDRC_PWRTMG_VAL     0x00408210
#define DDRC_RFSHTMG_VAL    0x007f8089
#define DDRC_CRCPARCTL2_VAL 0x0040051f
#define DDRC_INIT0_VAL      0x00020102
#define DDRC_INIT1_VAL      0x00020000
#define DDRC_INIT2_VAL      0x00002205
#define DDRC_INIT3_VAL      0x07300301
#define DDRC_INIT4_VAL      0x00200200
#define DDRC_INIT5_VAL      0x00210004
#define DDRC_INIT6_VAL      0x000006c0
#define DDRC_INIT7_VAL      0x08190000
#define DDRC_DRAMTMG0_VAL   0x11102311
#define DDRC_DRAMTMG1_VAL   0x00040419
#define DDRC_DRAMTMG2_VAL   0x0708060d
#define DDRC_DRAMTMG3_VAL   0x0050400c
#define DDRC_DRAMTMG4_VAL   0x08030409
#define DDRC_DRAMTMG5_VAL   0x06060403
#define DDRC_DRAMTMG6_VAL   0x01010004
#define DDRC_DRAMTMG7_VAL   0x00000606
#define DDRC_DRAMTMG8_VAL   0x03030d06
#define DDRC_DRAMTMG9_VAL   0x0002030b
#define DDRC_DRAMTMG11_VAL  0x1107010e
#define DDRC_DRAMTMG12_VAL  0x00020608
#define DDRC_ZQCTL1_VAL     0x0201908b
#define DDRC_DFITMG0_VAL    0x048b820b
#define DDRC_DFITMG1_VAL    0x00030304
#define DDRC_DFITMG2_VAL    0x00000909
#define DDRC_ADDRMAP6_VAL   0x0f070707
#define DDRC_ADDRMAP7_VAL   0x00000f0f
#define DDRC_ADDRMAP8_VAL   0x00001f01
#define DDRC_PERFWR1_VAL    0xffff3dc9

/* DDR physical interface parameters */
#define DDR_PHY_PGCR2_VAL        0x00f0fc08
#define DDR_PHY_PTR0_VAL         0x41a20d10
#define DDR_PHY_PTR1_VAL         0xcd141275
#define DDR_PHY_PLLCR0_VAL       0x01100000
#define DDR_PHY_DSGCR_VAL        0x02a04161
#define DDR_PHY_GPR1_VAL         0x000000de
#define DDR_PHY_DTPR0_VAL        0x07220f08
#define DDR_PHY_DTPR1_VAL        0x28200008
#define DDR_PHY_DTPR4_VAL        0x01112b07
#define DDR_PHY_DTPR5_VAL        0x00320f08
#define DDR_PHY_DTPR6_VAL        0x00000e0f
#define DDR_PHY_RDIMMCR1_VAL     0x00000200
#define DDR_PHY_MR0_VAL          0x00000630
#define DDR_PHY_MR2_VAL          0x00000020
#define DDR_PHY_MR4_VAL          0x00000000
#define DDR_PHY_BISTLSR_VAL      0x12340800
#define DDR_PHY_ZQCR_VAL         0x008aaa58
#define DDR_PHY_DX0GCR4_VAL      0x0e00b03c
#define DDR_PHY_DX1GCR4_VAL      0x0e00b03c
#define DDR_PHY_DX2GCR4_VAL      0x0e00b03c
#define DDR_PHY_DX3GCR4_VAL      0x0e00b03c
#define DDR_PHY_DX8SL0PLLCR0_VAL 0x01100000
#define DDR_PHY_DX8SL1PLLCR0_VAL 0x01100000
#define DDR_PHY_DX8SL2PLLCR0_VAL 0x01100000
#define DDR_PHY_DX8SL3PLLCR0_VAL 0x01100000
#define DDR_PHY_DX8SL4PLLCR0_VAL 0x21100000

#endif
