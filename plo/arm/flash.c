/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Flash driver
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

#include "flash.h"
#include "lut.h"
#include "config.h"
#include "errors.h"
#include "MVF50GS10MK50.h"

#define QuadSPI0_FLASH_BASE_ADDR 0x20000000
#define QuadSPI1_FLASH_BASE_ADDR 0x50000000

#define SEQID_WREN    0
#define SEQID_DDRQIOR 1
#define SEQID_QIOR    2
#define SEQID_QPP     3
#define SEQID_PP      4
#define SEQID_RDSR1   5
#define SEQID_RDCR    6
#define SEQID_WRR     7
#define SEQID_BE      8
#define SEQID_SE      9
#define SEQID_P4E    10
#define SEQID_RDID   11
#define SEQID_CLSR   12
#define SEQID_DIOR   13

#define RX_BUF_SIZE 0x80
#define CFI_TX_BUF_SIZE 6
#define TX_BUF_SIZE (1 << CFI_TX_BUF_SIZE)
#define DEFAULT_TRANSFER_SIZE 0

#ifdef FLASH_S25FL128S
	#define FLASH_STATUS_MASK  0x61000000
	#define FLASH_STATUS_READY 0x00000000
	#define FLASH_STATUS_BUSY  0x01000000

	#define SEQID_CFI_READ        SEQID_RDID
	#define SEQID_WRITE_ENABLE    SEQID_WREN
	#define SEQID_READ_STATUS     SEQID_RDSR1
	#define SEQID_FLASH_CONFIGURE SEQID_WRR
	#define SEQID_PROGRAM_PAGE    SEQID_QPP
	#define SEQID_READ            SEQID_DDRQIOR
	#define SEQID_ERASE1          SEQID_P4E
	#define SEQID_ERASE2          SEQID_SE
	#define SEQID_ERASE_UNIFORM   SEQID_SE
	#define SEQID_ERASE_ALL       SEQID_BE
	#define SEQID_CLEAR_ERRORS    SEQID_CLSR

	#define ERASE_CHANGE 0x20000
	#define INIT_FLASH_CONFIG 0x00020000 /* enable flash quad mode */
	#define INIT_QUADSPI_DDR
#endif
   

#ifdef FLASH_S25FL132K
	#define FLASH_STATUS_MASK  0x01000000
	#define FLASH_STATUS_READY 0x00000000
	#define FLASH_STATUS_BUSY  0x01000000

	#define SEQID_CFI_READ        SEQID_RDID
	#define SEQID_WRITE_ENABLE    SEQID_WREN
	#define SEQID_READ_STATUS     SEQID_RDSR1
	#define SEQID_FLASH_CONFIGURE SEQID_WRR
	#define SEQID_PROGRAM_PAGE    SEQID_PP
	#define SEQID_READ            SEQID_QIOR
	#define SEQID_ERASE1          SEQID_P4E
	#define SEQID_ERASE2          SEQID_SE
	#define SEQID_ERASE_UNIFORM   SEQID_SE
	#define SEQID_ERASE_ALL       SEQID_BE
	#define SEQID_CLEAR_ERRORS    SEQID_CLSR

	#define ERASE_CHANGE 0x20000
	#define INIT_FLASH_CONFIG 0x00020000 /* enable flash quad mode */
#endif


#ifdef FLASH_S25FL032P
	#define FLASH_STATUS_MASK  0x61000000
	#define FLASH_STATUS_READY 0x00000000
	#define FLASH_STATUS_BUSY  0x01000000

	#define SEQID_CFI_READ        SEQID_RDID
	#define SEQID_WRITE_ENABLE    SEQID_WREN
	#define SEQID_READ_STATUS     SEQID_RDSR1
	#define SEQID_FLASH_CONFIGURE SEQID_WRR
	#define SEQID_PROGRAM_PAGE    SEQID_QPP
	#define SEQID_READ            SEQID_QIOR
	#define SEQID_ERASE1          SEQID_P4E
	#define SEQID_ERASE2          SEQID_SE
	#define SEQID_ERASE_UNIFORM   SEQID_SE
	#define SEQID_ERASE_ALL       SEQID_BE
	#define SEQID_CLEAR_ERRORS    SEQID_CLSR

	#define ERASE_CHANGE 0x20000
	#define INIT_FLASH_CONFIG 0x00020000 /* enable flash quad mode */
#endif

#ifdef FLASH_S25FL127S
	#define FLASH_STATUS_MASK  0x01000000
	#define FLASH_STATUS_READY 0x00000000
	#define FLASH_STATUS_BUSY  0x01000000

	#define SEQID_CFI_READ        SEQID_RDID
	#define SEQID_WRITE_ENABLE    SEQID_WREN
	#define SEQID_READ_STATUS     SEQID_RDSR1
	#define SEQID_FLASH_CONFIGURE SEQID_WRR

#ifdef FLASH_MAX_DUAL_SPI
	#define SEQID_PROGRAM_PAGE    SEQID_PP
	#define SEQID_READ            SEQID_DIOR
#else
	#define SEQID_PROGRAM_PAGE    SEQID_QPP
	#define SEQID_READ            SEQID_QIOR
#endif

	#define SEQID_ERASE1          SEQID_P4E
	#define SEQID_ERASE2          SEQID_SE
	#define SEQID_ERASE_UNIFORM   SEQID_SE
	#define SEQID_ERASE_ALL       SEQID_BE
	#define SEQID_CLEAR_ERRORS    SEQID_CLSR

	#define ERASE_CHANGE 0x10000
#ifdef FLASH_MAX_DUAL_SPI
	#define INIT_FLASH_CONFIG 0x00000000 /* disable flash quad mode */
#else
	#define INIT_FLASH_CONFIG 0x00020000 /* enable flash quad mode */
#endif
#endif

static inline u32 _quadspi_isBusy(QuadSPI_Type *quadspi)
{
	return (quadspi->SR & (QuadSPI_SR_AHBGNT_MASK | QuadSPI_SR_AHB_ACC_MASK | QuadSPI_SR_IP_ACC_MASK));
}

static inline void _quadspi_doIpSeq(QuadSPI_Type *quadspi, u32 seqid, u16 transferSize)
{
	quadspi->IPCR = seqid << 24 | transferSize;
}

static inline void _quadspi_setFlashAddr(QuadSPI_Type *quadspi, u64 addr)
{
	quadspi->SFAR = (u32) addr;
}

static inline u16 _quadspi_RXfillLevel(QuadSPI_Type *quadspi)
{
	return (quadspi->RBSR & QuadSPI_RBSR_RDBFL_MASK) >> QuadSPI_RBSR_RDBFL_SHIFT;
}

static inline void _quadspi_clrRX(QuadSPI_Type *quadspi)
{
	quadspi->MCR |= QuadSPI_MCR_CLR_RXF_MASK;
}

static inline void _quadspi_clrTX(QuadSPI_Type *quadspi)
{
	quadspi->MCR |= QuadSPI_MCR_CLR_TXF_MASK;
	quadspi->FR = QuadSPI_FR_TBUF_MASK;
}

inline static void _quadspi_LUTunlock(QuadSPI_Type *quadspi)
{
	quadspi->LUTKEY = LUT_MAGIC_KEY;
	quadspi->LCKCR = QuadSPI_LCKCR_UNLOCK_MASK;
}

inline static void _quadspi_LUTlock(QuadSPI_Type *quadspi)
{
	quadspi->LUTKEY = LUT_MAGIC_KEY;
	quadspi->LCKCR = QuadSPI_LCKCR_LOCK_MASK;
}

static void _quadspi_LUTfill(QuadSPI_Type *quadspi)
{
	_quadspi_LUTunlock(quadspi);


	quadspi->LUT[(SEQID_DDRQIOR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)     |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_DDRQIOR)
										   | QuadSPI_LUT_INSTR1(LUT_CMD_ADDR_DDR)|QuadSPI_LUT_PAD1(LUT_PAD4)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_DDRQIOR << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_MODE_DDR)|QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_MODE_ONCE)
										   | QuadSPI_LUT_INSTR1(LUT_CMD_DUMMY)   |QuadSPI_LUT_PAD1(LUT_PAD4)|QuadSPI_LUT_OPRND1(0x06);
	quadspi->LUT[(SEQID_DDRQIOR << 2) + 2] = QuadSPI_LUT_INSTR0(LUT_CMD_READ_DDR)|QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(0x80)
										   | QuadSPI_LUT_INSTR1(LUT_CMD_STOP)    |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_QIOR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_QIOR)
										| QuadSPI_LUT_INSTR1(LUT_CMD_ADDR) |QuadSPI_LUT_PAD1(LUT_PAD4)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_QIOR << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_MODE) |QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_MODE_ONCE)
										| QuadSPI_LUT_INSTR1(LUT_CMD_DUMMY)|QuadSPI_LUT_PAD1(LUT_PAD4)|QuadSPI_LUT_OPRND1(0x04);
	quadspi->LUT[(SEQID_QIOR << 2) + 2] = QuadSPI_LUT_INSTR0(LUT_CMD_READ) |QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(0x01)
										| QuadSPI_LUT_INSTR1(LUT_CMD_STOP) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_DIOR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_DIOR)
										| QuadSPI_LUT_INSTR1(LUT_CMD_ADDR) |QuadSPI_LUT_PAD1(LUT_PAD2)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_DIOR << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_MODE)|QuadSPI_LUT_PAD0(LUT_PAD2)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_MODE_ONCE)
										| QuadSPI_LUT_INSTR1(LUT_CMD_READ) |QuadSPI_LUT_PAD1(LUT_PAD2)|QuadSPI_LUT_OPRND1(0x01);
	quadspi->LUT[(SEQID_DIOR << 2) + 2] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_WREN << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)   |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_WREN)
										| QuadSPI_LUT_INSTR1(LUT_CMD_STOP)  |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_QPP << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_QPP)
									   | QuadSPI_LUT_INSTR1(LUT_CMD_ADDR) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_QPP << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_WRITE)|QuadSPI_LUT_PAD0(LUT_PAD4)|QuadSPI_LUT_OPRND0(0x01)
									   | QuadSPI_LUT_INSTR1(LUT_CMD_STOP) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_PP << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_PP)
									  | QuadSPI_LUT_INSTR1(LUT_CMD_ADDR) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_PP << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_WRITE)|QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x01)
									  | QuadSPI_LUT_INSTR1(LUT_CMD_STOP) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_RDSR1 << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_RDSR1)
										 | QuadSPI_LUT_INSTR1(LUT_CMD_READ)|QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x01);
	quadspi->LUT[(SEQID_RDSR1 << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP)|QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_RDCR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_RDCR)
										| QuadSPI_LUT_INSTR1(LUT_CMD_READ)|QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x01);
	quadspi->LUT[(SEQID_RDCR << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP)|QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_WRR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_WRR)
									   | QuadSPI_LUT_INSTR1(LUT_CMD_WRITE)|QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x02);
	quadspi->LUT[(SEQID_WRR << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_BE << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_BE)
									  | QuadSPI_LUT_INSTR1(LUT_CMD_STOP)|QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	quadspi->LUT[(SEQID_P4E << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_P4E)
									   | QuadSPI_LUT_INSTR1(LUT_CMD_ADDR)|QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_P4E << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP)|QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_SE << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD)  |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_SE)
									  | QuadSPI_LUT_INSTR1(LUT_CMD_ADDR) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(24);
	quadspi->LUT[(SEQID_SE << 2) + 1] = QuadSPI_LUT_INSTR0(LUT_CMD_STOP) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(0x00);


	quadspi->LUT[(SEQID_CLSR << 2) + 0] = QuadSPI_LUT_INSTR0(LUT_CMD_CMD) |QuadSPI_LUT_PAD0(LUT_PAD1)|QuadSPI_LUT_OPRND0(FLASH_SPANSION_CMD_CLSR)
										| QuadSPI_LUT_INSTR1(LUT_CMD_STOP) |QuadSPI_LUT_PAD1(LUT_PAD1)|QuadSPI_LUT_OPRND1(0x00);


	_quadspi_LUTlock(quadspi);
}

static void quadspi_getChars(u32 src, char *dest, unsigned int len)
{
	switch (len) {
		case 4:
			*(dest + 3) = src >> (8 * 0);
		case 3:
			*(dest + 2) = src >> (8 * 1);
		case 2:
			*(dest + 1) = src >> (8 * 2);
		case 1:
			*(dest + 0) = src >> (8 * 3);
		default:
			break;
	}
}

static int _quadspi_receive4FlashData(QuadSPI_Type *quadspi, u32 seqid, u32 *dest)
{
	_quadspi_clrRX(quadspi);
	_quadspi_doIpSeq(quadspi, seqid, DEFAULT_TRANSFER_SIZE);
	while (_quadspi_isBusy(quadspi));
	if (_quadspi_RXfillLevel(quadspi) != 1) return -1;
	*dest = quadspi->RBDR[0];
	return 0;
}

static inline u32 quadspi_reverseBytes(const u32 v)
{
	unsigned result;

	__asm__ volatile ("rev %0, %1" : "=r" (result)	: "r" (v));

	return result;
}

static void _quadspi_receiveFlashData(QuadSPI_Type *quadspi, u32 seqid, char *dest, unsigned int len)
{
	u32 field = 0;

	_quadspi_clrRX(quadspi);
	_quadspi_doIpSeq(quadspi, seqid, len);
	while(_quadspi_isBusy(quadspi));

	while (len > 4) {
		*((u32 *)dest) = quadspi_reverseBytes(quadspi->RBDR[field]);
		dest += 4;
		len -= 4;
		field++;
	}
	quadspi_getChars(quadspi->RBDR[field], dest, len);
}

static void _quadspi_clearErrors(QuadSPI_Type *quadspi)
{
	while(_quadspi_isBusy(quadspi));
	_quadspi_doIpSeq(quadspi, SEQID_CLEAR_ERRORS, 0);
}

static int _quadspi_getFlashStatus(QuadSPI_Type *quadspi, u32 flashAddress)
{
	u32 statusValue;
	int ret;

	while(_quadspi_isBusy(quadspi));
	_quadspi_setFlashAddr(quadspi, flashAddress);
	ret = _quadspi_receive4FlashData(quadspi, SEQID_READ_STATUS, &statusValue);

	if (ret != 0) return ret;
	if ((statusValue & FLASH_STATUS_MASK) == FLASH_STATUS_READY)
		return 0;
	else if ((statusValue & FLASH_STATUS_MASK) == FLASH_STATUS_BUSY)
		return ERR_BUSY;
	else {
		_quadspi_clearErrors(quadspi);
		return ERR_FLASH_IO;
	}
}

static int _quadspi_send4FlashData(QuadSPI_Type *quadspi, u32 seqid, u32 src)
{
	_quadspi_clrTX(quadspi);
	quadspi->TBDR = src;
	_quadspi_doIpSeq(quadspi, seqid, DEFAULT_TRANSFER_SIZE);
	return 0;
}

s32 flash_open(u16 fn, char *name, u32 flags) {
	if ((fn & 0x1) == 0) {
		switch (fn >> 1) {
			case 0:
				if (QuadSPI0->SFA1AD > QuadSPI0_FLASH_BASE_ADDR)
					return QuadSPI0_FLASH_BASE_ADDR;
				break;
			case 1:
				if (QuadSPI0->SFA2AD > QuadSPI0->SFA1AD)
					return QuadSPI0->SFA1AD;
				break;
			case 2:
				if (QuadSPI0->SFB1AD > QuadSPI0->SFA2AD)
					return QuadSPI0->SFA2AD;
				break;
			case 3:
				if (QuadSPI0->SFB2AD > QuadSPI0->SFB1AD)
					return QuadSPI0->SFB1AD;
				break;
		}
		return ERR_FLASH_IO;
	} else {
		if (QuadSPI1->SFA1AD > QuadSPI1_FLASH_BASE_ADDR)
			return QuadSPI1_FLASH_BASE_ADDR;
	}
	return ERR_FLASH_IO;
}

s32 flash_read(u16 fn, s32 handle, u64 *pos, u8 *buff, u32 len)
{
	u16 toRead;
	u64 offs = *pos;
	QuadSPI_Type *quadspi = QuadSPI0;

	while(_quadspi_isBusy(quadspi));
	while (len > 0) {
		if (len > RX_BUF_SIZE) {
			toRead = RX_BUF_SIZE;
		} else {
			toRead = len;
		}
		_quadspi_setFlashAddr(quadspi, (u64) handle + offs);
		_quadspi_receiveFlashData(quadspi, SEQID_READ, buff, toRead);
		len -= toRead;
		offs += toRead;
		buff += toRead;
	}
	return offs - *pos;
}

s32 flash_close(u16 fn, s32 handle) {
	return 0;
}

static int _quadspi_flashInit(QuadSPI_Type *quadspi, u32 flashAddress)
{
	int ret = 0;

#ifdef SEQID_FLASH_CONFIGURE
	while(_quadspi_isBusy(quadspi));
	_quadspi_setFlashAddr(quadspi, flashAddress);
	_quadspi_doIpSeq(quadspi, SEQID_WRITE_ENABLE, 0);
	while(_quadspi_isBusy(quadspi));
	if ((ret = _quadspi_send4FlashData(quadspi, SEQID_FLASH_CONFIGURE, INIT_FLASH_CONFIG)) != 0) return ret;
	while((ret = _quadspi_getFlashStatus(quadspi, flashAddress)) == ERR_BUSY);
#endif

	return ret;
}

static void _quadspi_configure(QuadSPI_Type *quadspi)
{
	/* (0xF << 16) - reserved bits that has to be 1 */
	quadspi->MCR = (0xF << 16) | QuadSPI_MCR_SWRSTSD_MASK | QuadSPI_MCR_SWRSTHD_MASK;
	quadspi->MCR |= QuadSPI_MCR_MDIS_MASK;
	quadspi->FR = 0xFFFFFFFF;
	quadspi->SMPR = 0x0;

#ifdef INIT_QUADSPI_DDR
	quadspi->MCR |= QuadSPI_MCR_DDR_EN_MASK;
	quadspi->SMPR = QuadSPI_SMPR_DDRSMP(0x3);
#endif
	quadspi->RBCT |= QuadSPI_RBCT_RXBRD_MASK;

	quadspi->MCR &= ~QuadSPI_MCR_MDIS_MASK;
	quadspi->MCR &= ~(QuadSPI_MCR_SWRSTSD_MASK | QuadSPI_MCR_SWRSTHD_MASK);
	quadspi->MCR |= QuadSPI_MCR_CLR_TXF_MASK | QuadSPI_MCR_CLR_RXF_MASK;
}

void flash_init(void)
{
	u32 base;

#if defined(FLASH_QUADSPI0_A1_SIZE) || defined(FLASH_QUADSPI0_A2_SIZE) || defined(FLASH_QUADSPI0_B1_SIZE) || defined(FLASH_QUADSPI0_B2_SIZE)
	CCM->CCGR2  |= CCM_CCGR2_CG4(1); /* Clock is on during all modes, except stop mode. */
	CCM->CSCMR1 |= CCM_CSCMR1_QSPI0_CLK_SEL(0x3);
	CCM->CSCDR3 |= CCM_CSCDR3_QSPI0_X4_DIV(0x0)|CCM_CSCDR3_QSPI0_X2_DIV_MASK|CCM_CSCDR3_QSPI0_DIV_MASK|CCM_CSCDR3_QSPI0_EN_MASK;
	#if defined(FLASH_QUADSPI0_A1_SIZE) || defined(FLASH_QUADSPI0_A2_SIZE)
		IOMUXC->SINGLE.PTD0 = 0x001030C3;
		IOMUXC->SINGLE.PTD2 = 0x001030C3;
		IOMUXC->SINGLE.PTD3 = 0x001030C3;
		IOMUXC->SINGLE.PTD4 = 0x001030C3;
		IOMUXC->SINGLE.PTD5 = 0x001030C3;
		#ifdef FLASH_QUADSPI0_A1_SIZE
			IOMUXC->SINGLE.PTD1 = 0x001030FF;
		#endif
		#ifdef FLASH_QUADSPI0_A2_SIZE
			IOMUXC->SINGLE.PTB6 = 0x001030FF;
		#endif
	#endif
	#if defined(FLASH_QUADSPI0_B1_SIZE) || defined(FLASH_QUADSPI0_B2_SIZE)
			IOMUXC->SINGLE.PTD7 = 0x001030C3;
			IOMUXC->SINGLE.PTD9 = 0x001030C3;
			IOMUXC->SINGLE.PTD10 = 0x001030C3;
			IOMUXC->SINGLE.PTD11 = 0x001030C3;
			IOMUXC->SINGLE.PTD12 = 0x001030C3;
		#ifdef FLASH_QUADSPI0_B1_SIZE
			IOMUXC->SINGLE.PTD8 = 0x001030FF;
		#endif
		#ifdef FLASH_QUADSPI0_B2_SIZE
			IOMUXC->SINGLE.PTB7 = 0x001030FF;
		#endif
	#endif
	_quadspi_LUTfill(QuadSPI0);
	_quadspi_configure(QuadSPI0);
#endif
#ifdef FLASH_QUADSPI1_A1_SIZE
	ccm->CCGR8  |= CCM_CCGR8_CG4(1); /* Clock is on during all modes, except stop mode. */
	ccm->CSCMR1 |= CCM_CSCMR1_QSPI1_CLK_SEL(0x3);
	ccm->CSCDR3 |= CCM_CSCDR3_QSPI1_X4_DIV(0x0)|CCM_CSCDR3_QSPI1_X2_DIV_MASK|CCM_CSCDR3_QSPI1_DIV_MASK|CCM_CSCDR3_QSPI1_EN_MASK;
	IOMUXC->SINGLE.PTA19 = 0x001030C3;
	IOMUXC->SINGLE.PTB0 = 0x001030FF;
	IOMUXC->SINGLE.PTB1 = 0x001030C3;
	IOMUXC->SINGLE.PTB2 = 0x001030C3;
	IOMUXC->SINGLE.PTB3 = 0x001030C3;
	IOMUXC->SINGLE.PTB4 = 0x001030C3;
	_quadspi_LUTfill(QuadSPI1);
	_quadspi_configure(QuadSPI1);
#endif

#if defined(FLASH_QUADSPI0_A1_SIZE) || defined(FLASH_QUADSPI0_A2_SIZE) || defined(FLASH_QUADSPI0_B1_SIZE) || defined(FLASH_QUADSPI0_B2_SIZE)
	while(_quadspi_isBusy(QuadSPI0));
	base = QuadSPI0_FLASH_BASE_ADDR;
	#ifdef FLASH_QUADSPI0_A1_SIZE
	base += FLASH_QUADSPI0_A1_SIZE;
	QuadSPI0->SFA1AD = base;
	if (_quadspi_flashInit(QuadSPI0, base - 1) != 0) {
		base -= FLASH_QUADSPI0_A1_SIZE;
		QuadSPI0->SFA1AD = base;
	}
	#else
	QuadSPI0->SFA1AD = base;
	#endif
	#ifdef FLASH_QUADSPI0_A2_SIZE
	base += FLASH_QUADSPI0_A2_SIZE;
	QuadSPI0->SFA2AD = base;
	if (_quadspi_flashInit(QuadSPI0, base - 1) != 0) {
		base -= FLASH_QUADSPI0_A2_SIZE;
		QuadSPI0->SFA2AD = base;
	}
	#else
	QuadSPI0->SFA2AD = base;
	#endif
	#ifdef FLASH_QUADSPI0_B1_SIZE
	base += FLASH_QUADSPI0_B1_SIZE;
	QuadSPI0->SFB1AD = base;
	if (_quadspi_flashInit(QuadSPI0, base - 1) != 0) {
		base -= FLASH_QUADSPI0_B1_SIZE;
		QuadSPI0->SFB1AD = base;
	}
	#else
	QuadSPI0->SFB1AD = base;
	#endif
	#ifdef FLASH_QUADSPI0_B2_SIZE
	base += FLASH_QUADSPI0_B2_SIZE;
	QuadSPI0->SFB2AD = base;
	if (_quadspi_flashInit(QuadSPI0, base - 1) != 0) {
		base -= FLASH_QUADSPI0_B2_SIZE;
		QuadSPI0->SFB2AD = base;
	}
	#else
	QuadSPI0->SFB2AD = base;
	#endif
#endif
#ifdef FLASH_QUADSPI1_A1_SIZE
	while(_quadspi_isBusy(QuadSPI1));
	base = QuadSPI1_FLASH_BASE_ADDR + FLASH_QUADSPI1_A1_SIZE;
	QuadSPI1->SFA1AD = base;
	QuadSPI1->SFA2AD = base;
	QuadSPI1->SFB1AD = base;
	QuadSPI1->SFB2AD = base;
	if (_quadspi_flashInit(QuadSPI1, base - 1) != 0) {
		base -= FLASH_QUADSPI0_A1_SIZE;
		QuadSPI1->SFA1AD = base;
		QuadSPI1->SFA2AD = base;
		QuadSPI1->SFB1AD = base;
		QuadSPI1->SFB2AD = base;
	}
#endif
}

void flash_done(void) {}
