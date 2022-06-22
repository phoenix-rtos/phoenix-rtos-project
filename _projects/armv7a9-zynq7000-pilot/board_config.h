/*
 * Phoenix-RTOS
 *
 * Board config for armv7a9-zynq7000-pilot
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

/* DDR configuration */
#define DDRC_DRAM_PARAM_REG0 0x0004285b
#define DDRC_DRAM_PARAM_REG1 0x44e458d3
#define DDRC_DRAM_PARAM_REG2 0x7282bce5
#define DDRC_DRAM_PARAM_REG3 0x270872d0
#define DDRC_DRAM_PARAM_REG4 0x0

#define DDRC_DRAM_ADDR_MAP_COL 0xfff00000
#define DDRC_DRAM_ADDR_MAP_ROW 0x0F666666

#define DDRC_PHY_CONFIG0 0x40000001
#define DDRC_PHY_CONFIG1 0x40000001
#define DDRC_PHY_CONFIG2 0x40000001
#define DDRC_PHY_CONFIG3 0x40000001

#define DDRC_PHY_INIT_RATIO0 0x0002a81f
#define DDRC_PHY_INIT_RATIO1 0x00029822
#define DDRC_PHY_INIT_RATIO2 0x00026c10
#define DDRC_PHY_INIT_RATIO3 0x00026013

#define DDRC_PHY_RD_DQS_CFG0 0x00000035
#define DDRC_PHY_RD_DQS_CFG1 0x00000035
#define DDRC_PHY_RD_DQS_CFG2 0x00000035
#define DDRC_PHY_RD_DQS_CFG3 0x00000035

#define DDRC_PHY_WR_DQS_CFG0 0x0000009f
#define DDRC_PHY_WR_DQS_CFG1 0x000000a2
#define DDRC_PHY_WR_DQS_CFG2 0x00000090
#define DDRC_PHY_WR_DQS_CFG3 0x00000093

#define DDRC_PHY_WE_CFG0 0x000000ff
#define DDRC_PHY_WE_CFG1 0x000000fb
#define DDRC_PHY_WE_CFG2 0x000000f0
#define DDRC_PHY_WE_CFG3 0x000000ed

#define DDRC_WR_DATA_SLV0 0x000000df
#define DDRC_WR_DATA_SLV1 0x000000e2
#define DDRC_WR_DATA_SLV2 0x000000d0
#define DDRC_WR_DATA_SLV3 0x000000d3

/* SPI configuration */
#define SPI0_SCLK -1
#define SPI0_MOSI -1
#define SPI0_MISO -1
#define SPI0_SS0  -1
#define SPI0_SS1  -1
#define SPI0_SS2  -1

#define SPI1_SCLK 12
#define SPI1_MOSI 10
#define SPI1_MISO 11
#define SPI1_SS0  8  /* Barometer */
#define SPI1_SS1  9  /* Magnetometer */
#define SPI1_SS2  13 /* Accelerometer and Gyroscope */

/* Motors Configuration */
#define PWM_MOTOR1 "/dev/pwm4"
#define PWM_MOTOR2 "/dev/pwm5"
#define PWM_MOTOR3 "/dev/pwm0"
#define PWM_MOTOR4 "/dev/pwm1"

#endif
