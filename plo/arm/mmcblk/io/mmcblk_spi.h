#ifndef MMCBLK_SPI_H
#define MMCBLK_SPI_H


#include <dev/mmcblk/mmcblk.h>

extern void mmcblk_spi_init(void *cardPtr);
extern void mmcblk_spi_deinit(void *cardPtr);
extern int mmcblk_spi_sendCommand(void *cardPtr, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size, void *admaDT);
extern MmcblkResponse_t mmcblk_spi_waitForResponse(void *cardPtr, int cmd);
extern int mmcblk_spi_transferWait(void *cardPtr);
extern int mmcblk_spi_waitBusy(void *cardPtr);
extern void *mmcblk_spi_setupDMA(void *cardPtr, void *bufptr, s32 bufsize, FreePtr **fp, char *htBuf);
extern void mmcblk_spi_freeDMA(FreePtr *fp);
extern int mmcblk_spi_switchHighSpeed(void *cardPtr, u32 baudrate);
extern int mmcblk_spi_setupBaudRate(void *cardPtr, u32 baudrate);
extern int mmcblk_spi_setupBusWidth(void *cardPtr, MmcblkBusWidth_t width);
extern int mmcblk_spi_setupEndian(void *cardPtr, MmcblkEndian_t endian);


#endif
