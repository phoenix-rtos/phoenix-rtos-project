/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * QuadSPI LUT (lookup table) instructions
 *
 * Copyright 2014 Phoenix Systems
 * Author: Katarzyna Baranowska
 *
 * This file is part of Phoenix-RTOS.
 *
 * Phoenix-RTOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Phoenix-RTOS kernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phoenix-RTOS kernel; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _QUADSPI_LUT_H_
#define _QUADSPI_LUT_H_

#define LUT_MAGIC_KEY 0x5AF05AF0

#define LUT_CMD_STOP        0
#define LUT_CMD_CMD         1
#define LUT_CMD_ADDR        2
#define LUT_CMD_DUMMY       3
#define LUT_CMD_MODE        4
#define LUT_CMD_MODE2       5
#define LUT_CMD_MODE4       6
#define LUT_CMD_READ        7
#define LUT_CMD_WRITE       8
#define LUT_CMD_JMP_ON_CS   9
#define LUT_CMD_ADDR_DDR    10
#define LUT_CMD_MODE_DDR    11
#define LUT_CMD_MODE2_DDR   12
#define LUT_CMD_MODE4_DDR   13
#define LUT_CMD_READ_DDR    14
#define LUT_CMD_WRITE_DDR   15
#define LUT_CMD_DATA_LEARN  16

#define LUT_PAD1  0
#define LUT_PAD2  1
#define LUT_PAD4  2
#define LUT_PAD8  3

#define FLASH_SPANSION_CMD_READ_ID    0x90
#define FLASH_SPANSION_CMD_REMS       0x90
#define FLASH_SPANSION_CMD_RDID       0x9F
#define FLASH_SPANSION_CMD_RES        0xAB
#define FLASH_SPANSION_CMD_RDSR1      0x05
#define FLASH_SPANSION_CMD_RDSR2      0x07
#define FLASH_SPANSION_CMD_RDCR       0x35
#define FLASH_SPANSION_CMD_WRR        0x01
#define FLASH_SPANSION_CMD_WRDI       0x04
#define FLASH_SPANSION_CMD_WREN       0x06
#define FLASH_SPANSION_CMD_CLSR       0x30
#define FLASH_SPANSION_CMD_READ       0x03
#define FLASH_SPANSION_CMD_4READ      0x13
#define FLASH_SPANSION_CMD_FAST_READ  0x0B
#define FLASH_SPANSION_CMD_4FAST_READ 0x0C
#define FLASH_SPANSION_CMD_DDRFR      0x0D
#define FLASH_SPANSION_CMD_4DDRFR     0x0E
#define FLASH_SPANSION_CMD_DOR        0x3B
#define FLASH_SPANSION_CMD_4DOR       0x3C
#define FLASH_SPANSION_CMD_QOR        0x6B
#define FLASH_SPANSION_CMD_4QOR       0x6C
#define FLASH_SPANSION_CMD_DIOR       0xBB
#define FLASH_SPANSION_CMD_4DIOR      0xBC
#define FLASH_SPANSION_CMD_DDRDIOR    0xBD
#define FLASH_SPANSION_CMD_4DDRDIOR   0xBE
#define FLASH_SPANSION_CMD_QIOR       0xEB
#define FLASH_SPANSION_CMD_4QIOR      0xEC
#define FLASH_SPANSION_CMD_DDRQIOR    0xED
#define FLASH_SPANSION_CMD_4DDRQIOR   0xEE
#define FLASH_SPANSION_CMD_PP         0x02
#define FLASH_SPANSION_CMD_4PP        0x12
#define FLASH_SPANSION_CMD_QPP        0x32
#define FLASH_SPANSION_CMD_4QPP       0x34
#define FLASH_SPANSION_CMD_PGSP       0x85
#define FLASH_SPANSION_CMD_PGRS       0x8A
#define FLASH_SPANSION_CMD_P4E        0x20
#define FLASH_SPANSION_CMD_4P4E       0x21
#define FLASH_SPANSION_CMD_BE         0x60
#define FLASH_SPANSION_CMD_SE         0xD8
#define FLASH_SPANSION_CMD_4SE        0xDC
#define FLASH_SPANSION_CMD_ERSP       0x75
#define FLASH_SPANSION_CMD_ERRS       0x7A

#define FLASH_SPANSION_MODE_REPEAT    0xA5
#define FLASH_SPANSION_MODE_ONCE      0xFF

#endif