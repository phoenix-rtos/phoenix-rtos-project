/*
 * Phoenix-RTOS
 *
 * Board config for armv7m7-imxrt117x-evkb
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


/* Clocks */
#define CLOCK_SYS_PLL1_ENABLE     1
#define CLOCK_SYS_PLL1DIV2_ENABLE 1 /* used by ENET */
/* #define CLOCK_SYS_PLL1DIV5_ENABLE 1 */


/* Peripherals */


#define UART_CONSOLE 1

/* UART */

#define UART1             1
#define UART1_BAUDRATE    115200
#define UART1_BUFSIZE     512
#define UART1_TX_PIN      ad_24
#define UART1_RX_PIN      ad_25
#define UART1_HW_FLOWCTRL 0
#define UART1_RTS_PIN     ad_27
#define UART1_CTS_PIN     ad_26


#define UART2             1
#define UART2_BAUDRATE    115200
#define UART2_BUFSIZE     512
#define UART2_TX_PIN      disp_b2_10
#define UART2_RX_PIN      disp_b2_11
#define UART2_HW_FLOWCTRL 0
#define UART2_RTS_PIN     disp_b2_13
#define UART2_CTS_PIN     disp_b2_12

/*
#define UART3             0
#define UART3_BAUDRATE    115200
#define UART3_BUFSIZE     512
#define UART3_TX_PIN      ad_30
#define UART3_RX_PIN      ad_31
#define UART3_HW_FLOWCTRL 0
#define UART3_RTS_PIN     sd_b2_08
#define UART3_CTS_PIN     sd_b2_07

#define UART4             0
#define UART4_BAUDRATE    115200
#define UART4_BUFSIZE     512
#define UART4_TX_PIN      disp_b1_06
#define UART4_RX_PIN      disp_b1_04
#define UART4_HW_FLOWCTRL 0
#define UART4_RTS_PIN     disp_b1_07
#define UART4_CTS_PIN     disp_b1_05

#define UART5             0
#define UART5_BAUDRATE    115200
#define UART5_BUFSIZE     512
#define UART5_TX_PIN      ad_28
#define UART5_RX_PIN      ad_29
#define UART5_HW_FLOWCTRL 0
#define UART5_RTS_PIN     sd_b2_10
#define UART5_CTS_PIN     sd_b2_09

#define UART6             0
#define UART6_BAUDRATE    115200
#define UART6_BUFSIZE     512
#define UART6_TX_PIN      emc_b1_40
#define UART6_RX_PIN      emc_b1_41
#define UART6_HW_FLOWCTRL 0
#define UART6_RTS_PIN     emc_b2_01
#define UART6_CTS_PIN     emc_b2_00

#define UART7             0
#define UART7_BAUDRATE    115200
#define UART7_BUFSIZE     512
#define UART7_TX_PIN      disp_b2_06
#define UART7_RX_PIN      disp_b2_07
#define UART7_HW_FLOWCTRL 0
#define UART7_RTS_PIN     ad_03
#define UART7_CTS_PIN     ad_02

#define UART8             0
#define UART8_BAUDRATE    115200
#define UART8_BUFSIZE     512
#define UART8_TX_PIN      disp_b2_08
#define UART8_RX_PIN      disp_b2_09
#define UART8_HW_FLOWCTRL 0
#define UART8_RTS_PIN     ad_05
#define UART8_CTS_PIN     ad_04

#define UART9             0
#define UART9_BAUDRATE    115200
#define UART9_BUFSIZE     512
#define UART9_TX_PIN      sd_b2_00
#define UART9_RX_PIN      sd_b2_01
#define UART9_HW_FLOWCTRL 0
#define UART9_RTS_PIN     sd_b2_03
#define UART9_CTS_PIN     sd_b2_02

#define UART10             0
#define UART10_BAUDRATE    115200
#define UART10_BUFSIZE     512
#define UART10_TX_PIN      ad_15
#define UART10_RX_PIN      ad_16
#define UART10_HW_FLOWCTRL 0
#define UART10_RTS_PIN     ad_35
#define UART10_CTS_PIN     ad_34

#define UART11             0
#define UART11_BAUDRATE    115200
#define UART11_BUFSIZE     512
#define UART11_TX_PIN      lpsr_08
#define UART11_RX_PIN      lpsr_09
#define UART11_HW_FLOWCTRL 0
#define UART11_RTS_PIN     lpsr_11
#define UART11_CTS_PIN     lpsr_10

#define UART12             0
#define UART12_BAUDRATE    115200
#define UART12_BUFSIZE     512
#define UART12_TX_PIN      lpsr_00
#define UART12_RX_PIN      lpsr_01
#define UART12_HW_FLOWCTRL 0
#define UART12_RTS_PIN     lpsr_04
#define UART12_CTS_PIN     lpsr_05
*/

/* SPI */

/*
#define SPI1      0
#define SPI1_SCK  ad_28
#define SPI1_SDO  ad_30
#define SPI1_SDI  ad_31
#define SPI1_PCS0 ad_29
#define SPI1_PCS1 ad_18
#define SPI1_PCS2 ad_19
#define SPI1_PCS3 ad_20

#define SPI2      0
#define SPI2_SCK  sd_b2_07
#define SPI2_SDO  sd_b2_09
#define SPI2_SDI  sd_b2_10
#define SPI2_PCS0 sd_b2_08
#define SPI2_PCS1 sd_b2_11
#define SPI2_PCS2 ad_22
#define SPI2_PCS3 ad_23

#define SPI3      0
#define SPI3_SCK  disp_b1_04
#define SPI3_SDO  disp_b1_06
#define SPI3_SDI  disp_b1_05
#define SPI3_PCS0 disp_b1_07
#define SPI3_PCS1 disp_b1_08
#define SPI3_PCS2 disp_b1_09
#define SPI3_PCS3 disp_b1_10

#define SPI4      0
#define SPI4_SCK  disp_b2_12
#define SPI4_SDO  disp_b2_14
#define SPI4_SDI  disp_b2_13
#define SPI4_PCS0 disp_b2_15
#define SPI4_PCS1 sd_b2_04
#define SPI4_PCS2 sd_b2_05
#define SPI4_PCS3 sd_b2_06

#define SPI5      0
#define SPI5_SCK  lpsr_02
#define SPI5_SDO  lpsr_04
#define SPI5_SDI  lpsr_05
#define SPI5_PCS0 lpsr_03
#define SPI5_PCS1 lpsr_06
#define SPI5_PCS2 lpsr_07
#define SPI5_PCS3 lpsr_08

#define SPI6      0
#define SPI6_SCK  lpsr_10
#define SPI6_SDO  lpsr_11
#define SPI6_SDI  lpsr_12
#define SPI6_PCS0 lpsr_09
#define SPI6_PCS1 lpsr_08
#define SPI6_PCS2 lpsr_07
#define SPI6_PCS3 lpsr_06
*/

#endif
