/*
 * Phoenix-RTOS
 *
 * Board config for armv7m7-imxrt106x-evk
 *
 * Copyright 2022, 2024 Phoenix Systems
 * Author: Lukasz Kosinski, Daniel Sawka
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

/* #define PSEUDODEV 1 */
#define BUILTIN_POSIXSRV 1


/* Peripherals */


#define UART_CONSOLE 1

/* UART */

#define UART1             1
#define UART1_BAUDRATE    115200
#define UART1_BUFSIZE     512
#define UART1_TX_PIN      ad_b0_12
#define UART1_RX_PIN      ad_b0_13
#define UART1_HW_FLOWCTRL 0
#define UART1_RTS_PIN     ad_b0_15
#define UART1_CTS_PIN     ad_b0_14

#define UART2             1
#define UART2_BAUDRATE    115200
#define UART2_BUFSIZE     512
#define UART2_TX_PIN      ad_b1_02
#define UART2_RX_PIN      ad_b1_03
#define UART2_HW_FLOWCTRL 0
#define UART2_RTS_PIN     ad_b1_01
#define UART2_CTS_PIN     ad_b1_00

/*
#define UART3             0
#define UART3_BAUDRATE    115200
#define UART3_BUFSIZE     512
#define UART3_TX_PIN      emc_13
#define UART3_RX_PIN      emc_14
#define UART3_HW_FLOWCTRL 0
#define UART3_RTS_PIN     emc_16
#define UART3_CTS_PIN     emc_15

#define UART4             0
#define UART4_BAUDRATE    115200
#define UART4_BUFSIZE     512
#define UART4_TX_PIN      emc_19
#define UART4_RX_PIN      emc_20
#define UART4_HW_FLOWCTRL 0
#define UART4_RTS_PIN     emc_18
#define UART4_CTS_PIN     emc_17

#define UART5             0
#define UART5_BAUDRATE    115200
#define UART5_BUFSIZE     512
#define UART5_TX_PIN      emc_23
#define UART5_RX_PIN      emc_24
#define UART5_HW_FLOWCTRL 0
#define UART5_RTS_PIN     emc_27
#define UART5_CTS_PIN     emc_28

#define UART6             0
#define UART6_BAUDRATE    115200
#define UART6_BUFSIZE     512
#define UART6_TX_PIN      emc_25
#define UART6_RX_PIN      emc_26
#define UART6_HW_FLOWCTRL 0
#define UART6_RTS_PIN     emc_29
#define UART6_CTS_PIN     emc_30

#define UART7             0
#define UART7_BAUDRATE    115200
#define UART7_BUFSIZE     512
#define UART7_TX_PIN      emc_31
#define UART7_RX_PIN      emc_32
#define UART7_HW_FLOWCTRL 0
#define UART7_RTS_PIN     sd_b1_07
#define UART7_CTS_PIN     sd_b1_06

#define UART8             0
#define UART8_BAUDRATE    115200
#define UART8_BUFSIZE     512
#define UART8_TX_PIN      emc_38
#define UART8_RX_PIN      emc_39
#define UART8_HW_FLOWCTRL 0
#define UART8_RTS_PIN     sd_b0_03
#define UART8_CTS_PIN     sd_b0_02
*/

/* SPI */

/*
#define SPI1      0
#define SPI1_SCK  sd_b0_00
#define SPI1_SDO  sd_b0_02
#define SPI1_SDI  sd_b0_03
#define SPI1_PCS0 sd_b0_01
#define SPI1_PCS1 emc_31
#define SPI1_PCS2 emc_40
#define SPI1_PCS3 emc_41

#define SPI2      0
#define SPI2_SCK  sd_b1_07
#define SPI2_SDO  sd_b1_08
#define SPI2_SDI  sd_b1_09
#define SPI2_PCS0 sd_b1_06
#define SPI2_PCS1 emc_14
#define SPI2_PCS2 sd_b1_10
#define SPI2_PCS3 sd_b1_11

#define SPI3      0
#define SPI3_SCK  ad_b0_00
#define SPI3_SDO  ad_b0_01
#define SPI3_SDI  ad_b0_02
#define SPI3_PCS0 ad_b0_03
#define SPI3_PCS1 ad_b0_04
#define SPI3_PCS2 ad_b0_05
#define SPI3_PCS3 ad_b0_06

#define SPI4      0
#define SPI4_SCK  b0_03
#define SPI4_SDO  b0_02
#define SPI4_SDI  b0_01
#define SPI4_PCS0 b0_00
#define SPI4_PCS1 b1_03
#define SPI4_PCS2 b1_02
#define SPI4_PCS3 b1_11
*/

#endif
