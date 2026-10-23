/*
 * Phoenix-RTOS
 *
 * Board config for armv7a7-imx6ull-evk
 *
 * Copyright 2022 Phoenix Systems
 * Author: Lukasz Kosinski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

/* Wi-Fi (lwip WHD HAL): SDIO on USDHC2 (CSI pads), WL_PWR_ON = GPIO3_IO09, WL_REG_ON = GPIO4_IO26 */
#define CONFIG_WIFI_USDHC_ADDR 0x2194000
#define CONFIG_WIFI_USDHC_IRQ  (32 + 23)
#define CONFIG_WIFI_USDHC_CLK  clk_usdhc2

#define CONFIG_USDHC_CLK_MUX_PAD mux_csi_vsync
#define CONFIG_USDHC_CLK_PAD     pad_csi_vsync
#define CONFIG_USDHC_CLK_MUX_VAL 1
#define CONFIG_USDHC_CLK_ISEL    0
#define CONFIG_USDHC_CMD_MUX_PAD mux_csi_hsync
#define CONFIG_USDHC_CMD_PAD     pad_csi_hsync
#define CONFIG_USDHC_CMD_MUX_VAL 1
#define CONFIG_USDHC_CMD_ISEL    0
#define CONFIG_USDHC_D0_MUX_PAD  mux_csi_d0
#define CONFIG_USDHC_D0_PAD      pad_csi_d0
#define CONFIG_USDHC_D0_MUX_VAL  1
#define CONFIG_USDHC_D0_ISEL     0
#define CONFIG_USDHC_D1_MUX_PAD  mux_csi_d1
#define CONFIG_USDHC_D1_PAD      pad_csi_d1
#define CONFIG_USDHC_D1_MUX_VAL  1
#define CONFIG_USDHC_D1_ISEL     0
#define CONFIG_USDHC_D2_MUX_PAD  mux_csi_d2
#define CONFIG_USDHC_D2_PAD      pad_csi_d2
#define CONFIG_USDHC_D2_MUX_VAL  1
#define CONFIG_USDHC_D2_ISEL     2
#define CONFIG_USDHC_D3_MUX_PAD  mux_csi_d3
#define CONFIG_USDHC_D3_PAD      pad_csi_d3
#define CONFIG_USDHC_D3_MUX_VAL  1
#define CONFIG_USDHC_D3_ISEL     0

#define CONFIG_WL_PWR_ON_PORT    3
#define CONFIG_WL_PWR_ON_PIN     9
#define CONFIG_WL_PWR_ON_MUX_PAD mux_lcd_d4
#define CONFIG_WL_PWR_ON_PAD     pad_lcd_d4

#define CONFIG_WL_REG_ON_PORT    4
#define CONFIG_WL_REG_ON_PIN     26
#define CONFIG_WL_REG_ON_MUX_PAD mux_csi_d5
#define CONFIG_WL_REG_ON_PAD     pad_csi_d5

#endif
