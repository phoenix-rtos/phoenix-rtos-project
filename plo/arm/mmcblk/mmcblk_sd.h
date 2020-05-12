#ifndef MMCBLK_SD_H
#define MMCBLK_SD_H

#include "mmcblk.h"

int mmcblk_sd_init(MmcblkCard_t *card);
void mmcblk_sd_deinit(MmcblkCard_t *card);
// int mmcblk_sd_inserted(MmcblkCard_t *card);
// int mmcblk_sd_switchHighSpeed(MmcblkCard_t *card);
// 
// int mmcblk_sd_write(MmcblkCard_t *card, offs_t offs, char *buff, unsigned int len);
int mmcblk_sd_read(MmcblkCard_t *card, u32 offs, char *headbuff, char *bodybuff, char *tailbuff, unsigned int len);


#endif
