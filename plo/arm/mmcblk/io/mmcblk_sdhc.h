#ifndef MMCBLK_SDHC_H
#define MMCBLK_SDHC_H

#include "../mmcblk.h"

#define MMCBLK_SDHC_MAX_BAUDRATE 33000000UL

extern int mmcblk_sdhc_init(void *cardPtr);
extern int mmcblk_sdhc_reset(void *cardPtr);
extern int mmcblk_sdhc_sendCommand(void *cardPtr, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size);
extern int mmcblk_sdhc_sendCommandWithTransfer(void *card, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size, void *headbuff, void *bodybuff, void *tailbuff, u32 bufsize);
extern void mmcblk_sdhc_waitForResponse(void *cardPtr, int cmd, MmcblkResponse_t *ret);
extern int mmcblk_sdhc_transferWait(void *cardPtr, u32 bytes);
extern int mmcblk_sdhc_waitBusy(void *cardPtr, u32 bytes);
// extern int mmcblk_sdhc_switchHighSpeed(void *cardPtr, u32 baudrate);
extern int mmcblk_sdhc_setupBaudRate(void *cardPtr, u32 baudrate);
extern int mmcblk_sdhc_setupBusWidth(void *cardPtr, MmcblkBusWidth_t width);
extern int mmcblk_sdhc_setupEndian(void *cardPtr, MmcblkEndian_t endian);

#endif
