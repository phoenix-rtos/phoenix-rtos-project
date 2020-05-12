#include "mmcblk.h"
#include "mmcblk_sd.h"
#include "io/mmcblk_sdhc.h"
#include "../plostd.h"
#include "../errors.h"

int mmcblk_sd_init(MmcblkCard_t *card) {
	int x;
	int timeout=0;
	int status=0;
	MmcblkResponse_t response;
	u32 baudrate;
	MmcblkSCRReg_t scr __attribute__((aligned (4)));
	LOG("SD memory card");

	card->port->ioOps.sendCommand(card, MMCBLK_COMM_ALL_SEND_CID, 0, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_ALL_SEND_CID, &response);
	status = mmcblk_evaluateResponse(&response);
	if(status != 0)
	{
		LOG("Failed to read CID sd card register");
		return ERR_MMC_IO;
	}

	card->CID = response.response.r2.cid;

	for(x=0;x < 4; ++x)
		LOG("CID %x", card->CID.cid[x]);
	LOG("Manufacturer ID: %x", MMCBLK_CID_GET_MID(card->CID));
	LOG("OEM ID: %x%x", (char) (MMCBLK_CID_GET_OID(card->CID)>>8), (char) MMCBLK_CID_GET_OID(card->CID));
	LOG("PNM ID: %c%c%c%c%c", (char) (MMCBLK_CID_GET_PNM(card->CID)>>32), (char) (MMCBLK_CID_GET_PNM(card->CID)>>24), (char) (MMCBLK_CID_GET_PNM(card->CID)>>16), (char) (MMCBLK_CID_GET_PNM(card->CID)>>8), (char) MMCBLK_CID_GET_PNM(card->CID));

	card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_RELATIVE_ADDR, 0, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_RELATIVE_ADDR, &response);
	status = mmcblk_evaluateResponse(&response);
	if(status != 0)
	{
		LOG("Failed to get RCA");
		return ERR_MMC_IO;
	}
	card->RCA = ((u32)response.response.r6.response & 0xFFFF0000);
	LOG("RCA: %x %x %x", card->RCA, card->RCA >> 16, (u32)response.response.r6.bits.RCA);

	while(!timeout)
	{
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_ALL_SEND_CID, 0, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_ALL_SEND_CID, &response);
		if(response.timeout)
		{
			timeout = 1;
			continue;
		}
		card->CID = response.response.r2.cid;

		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_RELATIVE_ADDR, 0, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_RELATIVE_ADDR, &response);
		if(response.timeout)
		{
			timeout = 1;
			continue;
		}
		card->RCA = ((u32)response.response.r6.response & 0xFFFF0000);
	}

	card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_CSD, card->RCA, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_CSD, &response);
	status = mmcblk_evaluateResponse(&response);
	if(status != 0)
	{
		LOG("Failed to get CSD");
		return ERR_MMC_IO;
	}

	card->CSD = response.response.r2.csd;
	LOG("CSD [0] [1] [2] [3] %x %x %x %x", card->CSD.csd20.csd[0], card->CSD.csd20.csd[1], card->CSD.csd20.csd[2], card->CSD.csd20.csd[3]);
	if(MMCBLK_CSD20_GET_CSD_STRUCTURE(card->CSD.csd20))
	{
		u64 capacity = MMCBLK_CSD20_GET_C_SIZE(card->CSD.csd20);
		capacity += 1;
		card->capacity =  (u32) (capacity << 9);
		LOG("CAPACITY: %d blocks", card->capacity);

	}
	else
	{
		u32 csize = MMCBLK_CSD10_GET_C_SIZE(card->CSD.csd10);
		u32 mult = (1 << (MMCBLK_CSD10_GET_C_SIZE_MULT(card->CSD.csd10) + 2));
		u32 bLen = (1 << MMCBLK_CSD10_GET_READ_BL_LEN(card->CSD.csd10));
		u32 capacity = ((u64)(csize + 1) * mult * bLen) >> 9;
		card->capacity = (u32) (capacity << 9);
		LOG("CAPACITY: %d blocks", card->capacity);

	}

	baudrate = MMCBLK_CSD_GET_BAUDRATE(card->CSD.csd10);

	LOG("Switching to full speed mode");
	card->baudRate = card->port->ioOps.setupBaudRate(card, (baudrate < MMCBLK_SD_MAX_FULLSPEED_BAUDRATE)?baudrate:MMCBLK_SD_MAX_FULLSPEED_BAUDRATE);
	card->speed = eSDFullSpeed;

	LOG("Full speed baudrate: %d", card->baudRate);
	/* deselecting card - timeout expected */
	card->port->ioOps.sendCommand(card, MMCBLK_COMM_SELECT_DESELECT_CARD, 0, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SELECT_DESELECT_CARD, &response);
	/* selecting a card */
	card->port->ioOps.sendCommand(card, MMCBLK_COMM_SELECT_DESELECT_CARD, card->RCA, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SELECT_DESELECT_CARD, &response);
	status = mmcblk_evaluateResponse(&response);
	if(status != 0)
	{
		LOG("Failed to select the card");
		return ERR_MMC_IO;
	}

	card->port->ioOps.sendCommand(card, MMCBLK_COMM_SET_BLOCKLEN, MMCBLK_BLOCK_LENGTH, 0, 0);
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SET_BLOCKLEN, &response);
	status = mmcblk_evaluateResponse(&response);
	if(status != 0)
	{
		LOG("Failed to setup block size");
		return ERR_MMC_IO;
	}

	{
		card->port->ioOps.setupEndian(card, eBigEndian);
		card->port->ioOps.sendCommandWithTransfer(card, MMCBLK_COMM_SEND_SCR, card->RCA, 1, sizeof(scr), NULL, &scr, NULL, sizeof(scr));
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_SCR, &response);
		status = mmcblk_evaluateResponse(&response);
		if(status != 0)
		{
			card->port->ioOps.reset(card);
			return ERR_MMC_IO;
		}
		if(card->port->ioOps.transferWait(card, sizeof(scr)) != 0) {
			LOG("Failed to get SCR register");
			return ERR_MMC_IO;
		}

		card->SCR = scr;

	}

	LOG("SCR register: %x%x", card->SCR.scr[0], card->SCR.scr[1]);
	LOG("SCR cmd support: %x", MMCBLK_SCR_GET_CMD_SUPPORT(card->SCR));
	LOG("SCR sd spec 3: %x", MMCBLK_SCR_GET_SD_SPEC3(card->SCR));
	LOG("SCR sd bus widths: %x", MMCBLK_SCR_GET_SD_BUS_WIDTHS(card->SCR));
	LOG("SCR sd security: %x", MMCBLK_SCR_GET_SD_SECURITY(card->SCR));
	LOG("SCR data stat after erase: %x", MMCBLK_SCR_GET_DATA_STAT_AFTER_ERASE(card->SCR));
	LOG("SCR sd spec: %x", MMCBLK_SCR_GET_SD_SPEC(card->SCR));
	LOG("SCR scr structure: %x", MMCBLK_SCR_GET_SCR_STRUCTURE(card->SCR));

	if(MMCBLK_SCR_GET_SD_BUS_WIDTHS(card->SCR) & (1 << 2))
	{
		LOG("Switching to 4-bit mode");
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SET_BUS_WIDTH, 0x2, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SET_BUS_WIDTH, &response);
		status = mmcblk_evaluateResponse(&response);
		if(status != 0)
		{
			LOG("Failed to switch into 4-bit mode");
			return 0;
		}
		card->port->ioOps.setupBusWidth(card, eMmcblkBusWidth4b);
	}

	card->port->ioOps.setupEndian(card, eLittleEndian);

	LOG("Card baudrate: %d", card->baudRate);
	return 0;


}

void mmcblk_sd_deinit(MmcblkCard_t *card) {
	assert(!"Not implemented");
}

// int mmcblk_sd_inserted(MmcblkCard_t *card) {
// 	assert(!"Not implemented");
// 	return 0;
// }

int mmcblk_sd_switchHighSpeed(MmcblkCard_t *card) {
	assert(!"Not implemented");
	return 0;
}

// int mmcblk_sd_write(MmcblkCard_t *card, offs_t offs, char *buff, unsigned int len) {
// 	s32 ret=0;
// 	FreePtr *fp;
// 	void *dmaDesc = NULL;
// 	u32 sectorNum=0;
// 	MmcblkResponse_t response;
// 	int status=0;
// 	u32 sizeHead=SIZE_CACHE_LINE - ((u32)buff & (SIZE_CACHE_LINE-1));
// 	u32 sizeTail=(u32)buff & (SIZE_CACHE_LINE-1);
// 	status=status;
// 
// 	/* head/tail buffer - for cache management purposes */
// 	char *ht=NULL;
// 	void *headFreePtr=NULL;
// 
// 	/* 4 - aligned buffer required, length - multiplicity of 512 */
// 	assert(!((u32)buff & 0x3) && !(len & (MMCBLK_BLOCK_LENGTH-1)));
// 	assert(!(offs & (MMCBLK_BLOCK_LENGTH-1)));
// 	if(offs > card->capacity)
// 		return -EINVAL;
// 	if(len == 0)
// 		return ret;
// 
// 	sectorNum = len >> 9;
// 
// 	ht = vm_dokmallocaligned(2 * SIZE_CACHE_LINE, SIZE_CACHE_LINE, &headFreePtr);
// 	if(ht == NULL) {
// 		assert(0);
// 		return -ENOMEM;
// 	}
// 
// 	dmaDesc = card->port->ioOps.setupDMA(card, buff, len, &fp, ht);
// 	if(dmaDesc == NULL) {
// 		vm_kfree(headFreePtr);
// 		assert(0);
// 		return -ENOMEM;
// 	}
// 
// 	hal_cpuFlushCache( (char *)(((u32) buff & ~(SIZE_CACHE_LINE-1))+SIZE_CACHE_LINE), len-SIZE_CACHE_LINE);
// 
// 	memcpy(ht, buff, sizeHead);
// 	if(sizeTail > 0)
// 		memcpy(ht+SIZE_CACHE_LINE, buff+len-sizeTail, sizeTail);
// 
// 	hal_cpuFlushCache(ht, SIZE_CACHE_LINE*2);
// 
// 	do {
// 		hal_cpuReschedule();
// 		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_STATUS, card->RCA, 0, 0, NULL);
// 		response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_STATUS);
// 	} while(response.response.r1.bits.CURRENT_STATE != 4);
// 
// 	if(len == MMCBLK_BLOCK_LENGTH) {
// 		card->port->ioOps.sendCommand(card, MMCBLK_COMM_WRITE_BLOCK, offs>>9, 1, MMCBLK_BLOCK_LENGTH, dmaDesc);
// 		response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_WRITE_BLOCK);
// 	}
// 	else {
// 		card->port->ioOps.sendCommand(card, MMCBLK_COMM_WRITE_MULTIPLE_BLOCK, offs>>9, sectorNum, MMCBLK_BLOCK_LENGTH, dmaDesc);
// 		response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_WRITE_MULTIPLE_BLOCK);
// 	}
// 	status = mmcblk_evaluateResponse(&response);
// 
// 	if(response.error || (status != EOK && status != -EBUSY && status != -ETIMEDOUT)) {
// 		LOG("Write cmd error: %d", status);
// 		assert(0);
// 		ret = -1;
// 	}
// 	else {
// 		ret = card->port->ioOps.transferWait(card);
// 		if(ret == EOK)
// 			ret = len;
// 		else {
// 			LOG("Write error");
// 			if(len > MMCBLK_BLOCK_LENGTH) {
// 				card->port->ioOps.sendCommand(card, MMCBLK_COMM_STOP_TRANSMISSION, 0, 0, 0, NULL);
// 				response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_STOP_TRANSMISSION);
// 				card->port->ioOps.waitBusy(card);
// 				/* TODO - calculate number of blocks written properly */
// 			}
// 			/* TODO - appropriate reset operation sdhc->SYSCTL |= SDHC_SYSCTL_RSTD_MASK; */
// 			card->port->ioOps.reset(card);
// 		}
// 	}
// 	card->port->ioOps.freeDMA(fp);
// 	vm_kfree(headFreePtr);
// 	return ret;
// }


int mmcblk_sd_read(MmcblkCard_t *card, u32 offs, char *headbuff, char *bodybuff, char *tailbuff, unsigned int len) {
	s32 ret=0;
	MmcblkResponse_t response;
	int status = 0;

	do {
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_STATUS, card->RCA, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_STATUS, &response);
	} while(response.response.r1.bits.CURRENT_STATE != 4);

	if((len == MMCBLK_BLOCK_LENGTH) && (headbuff == NULL) && (tailbuff == NULL)) {
		card->port->ioOps.sendCommandWithTransfer(card, MMCBLK_COMM_READ_SINGLE_BLOCK, offs >> 9, 1, MMCBLK_BLOCK_LENGTH, headbuff, bodybuff, tailbuff, len);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_READ_SINGLE_BLOCK, &response);
	} else if((len == 0) && (((headbuff != NULL) && (tailbuff == NULL)) || ((headbuff == NULL) && (tailbuff != NULL)))) {
		card->port->ioOps.sendCommandWithTransfer(card, MMCBLK_COMM_READ_SINGLE_BLOCK, offs >> 9, 0, MMCBLK_BLOCK_LENGTH, headbuff, bodybuff, tailbuff, len);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_READ_SINGLE_BLOCK, &response);
	} else {
		card->port->ioOps.sendCommandWithTransfer(card, MMCBLK_COMM_READ_MULTIPLE_BLOCK, offs >> 9, len >> 9, MMCBLK_BLOCK_LENGTH, headbuff, bodybuff, tailbuff, len);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_READ_MULTIPLE_BLOCK, &response);
	}
	status = mmcblk_evaluateResponse(&response);

	if(response.error || (status != 0 && status != ERR_BUSY)) {
		LOG("Read cmd error");
		ret = ERR_MMC_IO;
	}
	else {
		ret = card->port->ioOps.transferWait(card, len);
		if(ret == 0)
			ret = len;
		else {
			LOG("Read error");

			if((len > MMCBLK_BLOCK_LENGTH) || (headbuff != NULL) || (tailbuff != NULL)) {
				card->port->ioOps.sendCommand(card, MMCBLK_COMM_STOP_TRANSMISSION, 0, 0, 0);
				card->port->ioOps.waitForResponse(card, MMCBLK_COMM_STOP_TRANSMISSION, &response);
				card->port->ioOps.waitBusy(card, len);
				/* TODO - calculate number of blocks read properly */
			}
			/* TODO - appropriate reset operation sdhc->SYSCTL |= SDHC_SYSCTL_RSTD_MASK; */
			card->port->ioOps.reset(card);
		}
	}

	return ret;
}
