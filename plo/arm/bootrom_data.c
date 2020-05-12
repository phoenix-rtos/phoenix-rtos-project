/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Configuration data used by Vybrid Boot ROM code
 *
 * @copyright 2015 Phoenix Systems
 * @author: Katarzyna Baranowska <katarzyna.baranowska@phoesys.com>
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

#include "config.h"
#include "MVF50GS10MK50.h"
#include "lut.h"

#define  COMAND_SEQ_LEN            128 /**< interms of instruction word size */

typedef struct 
{
	u32 rsvd0;
	u8 hold_delay;
	u8 hsps; 
	u8 hsds; 
	u8 rsvd1;
	u32 rsvd[3];
	u32 cs_hold_time; /**< CS hold time in terms of serial clock.(for example 1 serial clock cyle) */
	u32 cs_setup_time; /**< CS setup time in terms of serial clock.(for example 1 serial clock cyle) */
	u32 sflash_A1_size; /**< interms of Bytes */
	u32 sflash_A2_size; /**< interms of Bytes */
	u32 sflash_B1_size; /**< interms of Bytes */
	u32 sflash_B2_size; /**< interms of Bytes */
	u32 sclk_freq; /**< In  00 - 18MHz, 01 - 60MHz, 02 - 74MHz, 03 - 99MHz (only for SDR Mode) */
	u32 rsvd5; /**< Reserved for Future Use */
	u8 sflash_type; /**< 01-Single,02--Dual 03--Quad */
	u8 sflash_port; /**< 0--Only Port-A,1--Both PortA and PortB */
	u8 ddr_mode_enable; /**< Enable DDR mode if set to TRUE */
	u8 dqs_enable; /**< Enable DQS mode if set to TRUE. */
	u8 parallel_mode_enable; /**< Enable Individual or parrallel mode. */
	u8 portA_cs1;
	u8 portB_cs1;
	u8 fsphs;
	u8 fsdly;
	u8 ddrsmp;
	u16 command_seq[COMAND_SEQ_LEN]; /**< set of seq to perform optimum read on SFLASH as as per vendor SFLASH */

	/* added to make even 0x400 size */
	/* this is required to pad the memory space when using DS-5 fromelf parser to create C-array from elf file */
	u8 empty[0x2C0];
} flash_conf_t;


typedef struct {
  u32 start;        /**< boot start location */
  u32 size;         /**< size */
  u32 plugin;       /**< plugin flag - 1 if downloaded application is plugin */
  u32 reserved;		/**< placehoder to make even 0x10 size */
} boot_data_t;


typedef struct {
    /** @ref hdr with tag #HAB_TAG_IVT, length and HAB version fields
     *  (see @ref data)
     */
    u32 hdr;
    /** Absolute address of the first instruction to execute from the
     *  image
     */
    u32 entry;
    /** Reserved in this version of HAB: should be NULL. */
    u32 reserved1;
    /** Absolute address of the image DCD: may be NULL. */
    u32 dcd;
    /** Absolute address of the Boot Data: may be NULL, but not interpreted
     *  any further by HAB
     */
    u32 boot_data;
    /** Absolute address of the IVT.*/
    u32 self;
    /** Absolute address of the image CSF.*/
    u32 csf;
    /** Reserved in this version of HAB: should be zero. */
    u32 reserved2;
} ivt_t;

#define IVT_MAJOR_VERSION           0x4
#define IVT_MAJOR_VERSION_SHIFT     0x4
#define IVT_MAJOR_VERSION_MASK      0xF
#define IVT_MINOR_VERSION           0x1
#define IVT_MINOR_VERSION_SHIFT     0x0
#define IVT_MINOR_VERSION_MASK      0xF

#define IVT_VERSION(major, minor)   \
  ((((major) & IVT_MAJOR_VERSION_MASK) << IVT_MAJOR_VERSION_SHIFT) |  \
  (((minor) & IVT_MINOR_VERSION_MASK) << IVT_MINOR_VERSION_SHIFT))


#define IVT_TAG_HEADER        (0xD1)       /**< Image Vector Table */
#define IVT_SIZE              (((sizeof(ivt_t) & 0x00ff) << 8) + ((sizeof(ivt_t) & 0xff00) >> 8))
#define IVT_PAR               IVT_VERSION(IVT_MAJOR_VERSION, IVT_MINOR_VERSION)
#define IVT_HEADER          (IVT_TAG_HEADER | (IVT_SIZE << 8) | (IVT_PAR << 24))


typedef struct {
	flash_conf_t quadspi_conf;
	ivt_t image_vector_table;
	boot_data_t boot_data;
}  bootrom_data_t;

extern const char _start[];
extern const char _ram_begin[];
extern const char _plo_img_size[];

const volatile bootrom_data_t __attribute__ ((section (".bootrom_data"))) bootrom_data = {
	.quadspi_conf = {
		.sflash_A1_size = 0x400000, /* Flash A1 Size in Bytes */
#ifdef FLASH_MAX_DUAL_SPI
		.sflash_type = 2, /**< 01-Single,02--Dual 04--Quad */
#else
		.sflash_type = 4, /**< 01-Single,02--Dual 04--Quad */
#endif
		.sflash_port = 0, /**< 0--Only Port-A,1--Both PortA and PortB */
		.ddr_mode_enable = 0, /* DDR Mode Enable (0=disabled, 1=enabled) */
		.dqs_enable = 0, /**< Enable DQS mode if set to TRUE. */
		.parallel_mode_enable = 0, /**< Enable Individual or parrallel mode. */
		.sclk_freq = 3, /**< In  00 - 18MHz, 01 - 60MHz, 02 - 74MHz, 03 - 99MHz (only for SDR Mode) */
		.command_seq =
#ifdef FLASH_MAX_DUAL_SPI
		/* LUT Programming (Dual IO ) */
		{QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_DIOR),
		 QuadSPI_LUT_INSTR0(LUT_CMD_ADDR) |QuadSPI_LUT_PAD0(LUT_PAD2)|QuadSPI_LUT_OPRND0(24),
		 QuadSPI_LUT_INSTR0(LUT_CMD_MODE) |QuadSPI_LUT_PAD0(LUT_PAD2)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_MODE_REPEAT),
		 QuadSPI_LUT_INSTR0(LUT_CMD_READ) |QuadSPI_LUT_PAD0(LUT_PAD2)|QuadSPI_LUT_OPRND0(0x80),
		 QuadSPI_LUT_INSTR0(LUT_CMD_JMP_ON_CS) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x01)}
#else
		/* LUT Programming (Quad IO) */
		{QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_QIOR),
		 QuadSPI_LUT_INSTR0(LUT_CMD_ADDR) |QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(24),
		 QuadSPI_LUT_INSTR0(LUT_CMD_MODE) |QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_MODE_REPEAT),
		 QuadSPI_LUT_INSTR0(LUT_CMD_DUMMY)|QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(0x04),
		 QuadSPI_LUT_INSTR0(LUT_CMD_READ) |QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(0x80),
		 QuadSPI_LUT_INSTR0(LUT_CMD_JMP_ON_CS) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x01)}
#endif
	},
	
	.image_vector_table = {
		.hdr = IVT_HEADER,                              /* IVT Header */
		.entry = (u32) _start,                          /* Image  Entry Function */
		.boot_data = (u32) &bootrom_data.boot_data,     /* Address where BOOT Data Structure is stored */
		.self = (u32) &bootrom_data.image_vector_table, /* Pointer to IVT Self (absolute address */
	},
	
	.boot_data = {
		.start = (u32) _ram_begin,
		.size  = (u32) _plo_img_size,
	},
};
