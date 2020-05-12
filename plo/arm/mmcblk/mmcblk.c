#include "mmcblk.h"
#include "mmcblk_sd.h"
#include "io/mmcblk_sdhc.h"
#include "../plostd.h"
#include "../errors.h"
#include "../timer.h"
#include "../config.h"
#include "../low.h"
#include "../MVF50GS10MK50.h"

#ifndef MMCBLK_CD_LEVEL
/* CARD DETECT LEVEL
 * 0 - active low
 * 1 - active high
 * 2 - ignored
 */
  #define MMCBLK_CD_LEVEL		2
#endif

static MmcblkPortDesc_t mmcblkPorts[] = {
	{
		.ioBase = (void *)SDHC1_BASE,
		.irq = SDHC1_IRQn,
		.ioOps = {
			.init = mmcblk_sdhc_init,
			.reset = mmcblk_sdhc_reset,
			.sendCommand = mmcblk_sdhc_sendCommand,
			.sendCommandWithTransfer = mmcblk_sdhc_sendCommandWithTransfer,
			.waitForResponse = mmcblk_sdhc_waitForResponse,
			.transferWait = mmcblk_sdhc_transferWait,
			.waitBusy = mmcblk_sdhc_waitBusy,
// 			.switchHighSpeed = mmcblk_sdhc_switchHighSpeed,
			.setupBaudRate = mmcblk_sdhc_setupBaudRate,
			.setupBusWidth = mmcblk_sdhc_setupBusWidth,
			.setupEndian = mmcblk_sdhc_setupEndian
		},
		.CSPort = -1,
		.CDPort = 0x86,
		.CDActive = MMCBLK_CD_LEVEL,
	}
};

#define MMCBL_PORTS_COUNT (sizeof(mmcblkPorts) / sizeof(MmcblkPortDesc_t))

static MmcblkCard_t card[MMCBL_PORTS_COUNT];

static struct MmcblkCardOps_t cardOps[eCardTypeNum] = {
	{
		.init = NULL,
		.deinit = NULL,
// 		.inserted = NULL,
// 		.switchHighSpeed = NULL,
// 		.write = NULL,
		.read = NULL
	},
	{
		.init = mmcblk_sd_init,
		.deinit = mmcblk_sd_deinit,
// 		.inserted = mmcblk_sd_inserted,
// 		.switchHighSpeed = mmcblk_sd_switchHighSpeed,
// 		.write = mmcblk_sd_write,
		.read = mmcblk_sd_read
	},
	{
		.init = NULL,
		.deinit = NULL,
// 		.inserted = NULL,
// 		.switchHighSpeed = NULL,
// 		.write = NULL,
		.read = NULL
	},
	{
		.init = NULL,
		.deinit = NULL,
// 		.inserted = NULL,
// 		.switchHighSpeed = NULL,
// 		.write = NULL,
		.read = NULL
	},
	{
		.init = NULL,
		.deinit = NULL,
// 		.inserted = NULL,
// 		.switchHighSpeed = NULL,
// 		.write = NULL,
		.read = NULL
	},
	{
		.init = NULL,
		.deinit = NULL,
// 		.inserted = NULL,
// 		.switchHighSpeed = NULL,
// 		.write = NULL,
		.read = NULL
	}
};


int mmcblk_evaluateResponse(MmcblkResponse_t *response)
{
	int status=0;
	if(response == NULL)
	{
		status=0;
		return status;
	}
	if(response->timeout)
	{
		status=ERR_TIMEOUT;
		return status;
	}
	else if(response->error)
	{
		status = ERR_MMC_IO;
		return status;
	}
	else if(response->busy)
	{
		status = ERR_BUSY;
		return status;
	}

	switch(response->responseType)
	{
		case MMCBLK_COMM_RESPONSE_R1:
			if(response->response.r1.bits.CARD_IS_LOCKED)
				LOG("CARD IS LOCKED!");
			if(response->response.r1.response & 0xFFFC0000)
			{
				LOG("FAULTY R1 RESPONSE: %x", response->response.r1.response );
				status = ERR_MMC_IO;
			}
		break;
		case MMCBLK_COMM_RESPONSE_NO_RESPONSE:
		case MMCBLK_COMM_RESPONSE_R1b:
		case MMCBLK_COMM_RESPONSE_R2:
		case MMCBLK_COMM_RESPONSE_R3:
		case MMCBLK_COMM_RESPONSE_R4:
		case MMCBLK_COMM_RESPONSE_R5:
		case MMCBLK_COMM_RESPONSE_R5b:
		case MMCBLK_COMM_RESPONSE_R6:
		break;
	}
	return status;
}


static int cardIdentify(MmcblkCard_t *card){
	MmcblkResponse_t response;
	MmcblkOCRReg_t ocr;
	LOG("Identifying card...");

	ocr = 0;
	ocr |= MMCBLK_OCR_V28_V29;
	ocr |= MMCBLK_OCR_V29_V30;
	ocr |= MMCBLK_OCR_V30_V31;

	card->label = eCardUnknown;
	/* CMD5, check SDIO */
	/* operation voltage, command argument is zero */

	{
		//0xDC is test pattern
		u32 arg = (1 << 8) | 0xDC;
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_IF_COND, arg, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_IF_COND, &response);
		if(response.timeout || response.busy)
		{
			LOG("Old card %x %x", response.timeout, response.busy);
		}
		else
		{
			LOG("New card");

			if(response.response.r7.response == arg)
			{
				LOG("Pattern match");
			}
			else
			{
				LOG("Pattern does not match! %x", response.response.r7.response);
				return ERR_MMC_IO;
			}
			ocr |= MMCBLK_OCR_CCS;
		}
	}
/*
	card->port->ioOps.sendCommand(card, MMCBLK_COMM_IO_SEND_OP_COND, 0, 0, 0, NULL);
	response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_IO_SEND_OP_COND);
	if (!response.timeout && !response.error) {
		if( ((response.response.r4.ocr >> 28) & 0x07) && (response.response.r4.ocr & 0x300000) )
		{
			int ov = 0;
			do
			{
				proc_threadSleep(100);
				++ov;
				card->port->ioOps.sendCommand(card, MMCBLK_COMM_IO_SEND_OP_COND, 0x300000, 0, 0, NULL);
				response = card->port->ioOps.waitForResponse(card, MMCBLK_COMM_IO_SEND_OP_COND);
				if(response.error || response.busy)
					return ERR_MMC_IO;
			}
			while(!!(response.response.r4.ocr & 0x88000000) && (ov <= 255));
			if(response.response.r4.ocr & 0x80000000)
			{
				card->label = eCardSDIO;
			}
			if(response.response.r4.ocr & 0x08000000)
			{
				if(card->label == eCardSDIO)
					card->label = eCardSDCombo;
				else
					card->label = eCardSD;
			}
		}
	}
	else
	{
		card->port->ioOps.reset(card);
	}
*/
	if(card->label == eCardSDIO)
		return 0;

	/* FROM SD SPECIFICATION https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf
	  ACMD55 does not exist. If multiple CMD55 are issued continuously, APP_CMD bit in each
	  response is set to 1. The command issued immediately after the last CMD55 shall be
	  interpreted as ACMD. When more than one command (except CMD55) is issued directly
	  after CMD55, the first command is interpreted as ACMD and the following commands
	  are interpreted as regular commands
	*/

	// CMD55, Application specific
	card->port->ioOps.sendCommand(card, MMCBLK_COMM_APP_CMD, 0, 0, 0);//RCA is unkonwn yet
	card->port->ioOps.waitForResponse(card, MMCBLK_COMM_APP_CMD, &response);
	if (0 == mmcblk_evaluateResponse(&response)) {//no error occured
		u32 arg = (ocr & 0x00FFFFFF)  | (ocr & MMCBLK_OCR_CCS);
		/* CMD55 is accepted */
		LOG("CMD55 accepted - SD or COMBO CARD");

		/* ACMD41, to set voltage range for memory part or SD card */
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SD_APP_OP_COND, 0, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SD_APP_OP_COND, &response);
		LOG("Card OP cond: %x", response.response.r3.ocr);
		if((response.response.r3.ocr & MMCBLK_OCR_V33_V34) || (response.response.r3.ocr & MMCBLK_OCR_V32_V33))
		{
			int ov = 0;
			do
			{
				timer_wait(1, TIMER_EXPIRE, NULL, 0, NULL);
				++ov;
				card->port->ioOps.sendCommand(card, MMCBLK_COMM_SD_APP_OP_COND, arg, 0, 0);
				card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SD_APP_OP_COND, &response);
				if(response.error || response.timeout)
				{
					LOG("Sd card init error");
					return ERR_MMC_IO;
				}
			}while(!(response.response.r3.ocr & MMCBLK_OCR_CPUPS) && ov < 255);


			if (card->label == eCardUnknown)
			{
				card->label = eCardSD;
			}
		}
		else
		{
			card->label = eCardUnsupported;
		}

		return 0;
	}
	else if (!response.timeout) {
		/* command/response pair is corrupted */
		LOG("CMD_APP failed");
		return ERR_MMC_IO;
	}
	else {
		// CMD55 is refuse, it must be MMC card or CE-ATA card
		LOG("CMD55 refused - MMC or CE-ATA");
		if (card->label == eCardSDCombo) {
			// change label
			card->label = eCardSDIO;
			LOG("Card is SDIO, not COMBO");
			//ignore the error or report it;
			// card is identified as SDIO card
			return 0;
		}
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_SEND_OP_COND, ocr, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_SEND_OP_COND, &response);
		if(response.timeout)
		{ // CMD1 is not accepted, either
			card->label = eCardUnknown;
			LOG("CMD1 not accepted - card unknown");
			//label the card as UNKNOWN;
			return -1;
		}

		//TODO - check for CE-ATA signature succeeded
		if (0)
		{
			// the card is CE-ATA
			LOG("Detected CE-ATA card");
			// store CE-ATA specific info from the signature;
			card->label = eCardCEATA;
			//label the card as CE-ATA;
		} // of if (check for CE-ATA ...
		else
		{
			LOG("Detected MMC card");
			card->label = eCardMMC;
		}
	}
	return 0;
}

static const GPIO_Type *GPIOs[] = GPIO_BASES;

static int mmcblk_cardInit(MmcblkCard_t *card){
	int inserted;
	int status = 0;
	MmcblkResponse_t res;

	if(card->port->CDActive == 2)
		inserted = 1;
	else {
		inserted = (!! ((GPIOs[card->port->CDPort / 32]->PDIR >> (card->port->CDPort % 32)) & 1)) == (!! card->port->CDActive);
	}

	if(inserted)
	{
		LOG("Mmcblk card init");
		card->port->ioOps.reset(card);
		card->port->ioOps.setupBusWidth(card, eMmcblkBusWidth1b);
		card->port->ioOps.setupEndian(card, eLittleEndian);
		card->port->ioOps.setupBaudRate(card, MMCBLK_MAX_BASIC_BAUDRATE);

		/* reset the card with CMD0 */
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_GO_IDLE_STATE, 0, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_GO_IDLE_STATE, &res);
		LOG("Card in idle state");
		timer_wait(100, TIMER_EXPIRE, NULL, 0, NULL);
		status = cardIdentify(card);
		if(status != 0 ) {
			return status;
		} else if (card->label == eCardUnknown || card->label == eCardUnsupported || card->label == eCardSDCombo) {
			plostd_printf(ATTR_ERROR, "MMCBLK: Card not identified or unsuppotred card detected\n");
			return ERR_UNSUPPORTED;
		} else {
			LOG("Card identification success");
			assert(card->label < eCardTypeNum && card->label > 0);
			card->cardOps = &cardOps[card->label];
			if(0 == card->cardOps->init(&(card[0])))
			{
				LOG("Card initialized");
				return 1;
			}
			else
			{
				plostd_printf(ATTR_ERROR, "MMCBLK: Card initialization failure\n");
				return ERR_MMC_IO;
			}
		}
	}
	return 0;
}

static int mmcblk_cardReinit(MmcblkCard_t *card){
	int inserted = 0;
	int status = 0;
	MmcblkResponse_t res;
	timer_wait(100, TIMER_EXPIRE, NULL, 0, NULL);

	if(card->port->CDActive == 2)
		inserted = 1;
	else {
		inserted = (!! ((GPIOs[card->port->CDPort / 32]->PDIR >> (card->port->CDPort % 32)) & 1)) == (!! card->port->CDActive);
	}

	if(inserted)
	{
		LOG("Mmcblk card reinit: %d", cardIdx);
		card->port->ioOps.reset(card);
		card->port->ioOps.setupBusWidth(card, eMmcblkBusWidth1b);
		card->port->ioOps.setupEndian(card, eLittleEndian);
		card->port->ioOps.setupBaudRate(card, MMCBLK_MAX_BASIC_BAUDRATE);

		/* reset the card with CMD0 */
		card->port->ioOps.sendCommand(card, MMCBLK_COMM_GO_IDLE_STATE, 0, 0, 0);
		card->port->ioOps.waitForResponse(card, MMCBLK_COMM_GO_IDLE_STATE, &res);
		LOG("Card in idle state");
		timer_wait(100, TIMER_EXPIRE, NULL, 0, NULL);

		status = cardIdentify(card);
		if(status != 0 || card->label == eCardUnknown || card->label == eCardUnsupported)
			LOG("Card identification failure");
		else {
			LOG("Card identification success");
			assert(card->label < eCardTypeNum && card->label > 0);
			if(card->label != eCardSDCombo)
			{
				card->cardOps = &cardOps[card->label];
				if(0 == card->cardOps->init(card))
				{
					LOG("Card initialized");
				}
				else
				{
					LOG("Card initialization failure");
					return ERR_MMC_IO;
				}
			}
			else
			{
				assert(!"COMBO card support Not implemented");
			}
		}
	}
	else {
		return ERR_ARG;
	}
	return 0;
}

void  cardDeInit(MmcblkCard_t *card) {
	card->baudRate = 0;
	card->capacity = 0;
	card->busWidth = eMmcblkBusWidth1b;
	low_memset(&card->CID, 0x0, sizeof(&card->CID));
	low_memset(&card->CSD, 0x0, sizeof(&card->CSD));
	low_memset(&card->SCR, 0x0, sizeof(&card->SCR));
	low_memset(&card->OCR, 0x0, sizeof(&card->OCR));
	low_memset(&card->RCA, 0x0, sizeof(&card->RCA));
	card->label = eCardUnknown;
	card->eventReg = 0;
	card->speed = eBasicSpeed;
	card->voltage = 0;
	card->sectorSize = 512;
}

static int mmcblk_totalCardReset(MmcblkCard_t *card) {
	int ret = 0;
	LOG("SD card reset\n");
	cardDeInit(card);
	card->cardOps->deinit(card);
	ret = mmcblk_cardReinit(card);
	return ret;
}

s32 mmcblk_read(u16 bn, s32 handle, u64 *offs_ptr, u8 *buff, u32 len)
{
	unsigned int head_len, tail_len;
	char head[MMCBLK_BLOCK_LENGTH] __attribute__(( aligned(4) ));
	static char tail[MMCBLK_BLOCK_LENGTH] __attribute__(( aligned(4) ));
	static u32 tail_addr = -1;
	char *head_ptr = head;
	char *tail_ptr = tail;
	int ret, retry = 5;
	u32 offs = (u32)*offs_ptr;

	if (handle != bn)
		return ERR_ARG;

	if((u32) buff & 0x3U) {
		plostd_printf(ATTR_ERROR, "MMCBLK: Improper read buffer alignment\n");
		return ERR_ARG;
	}
	
	if (len == 0)
		return 0;

	head_len = offs % MMCBLK_BLOCK_LENGTH;
	tail_len = (len + head_len) % MMCBLK_BLOCK_LENGTH;
	if (head_len == 0) {
		head_ptr = NULL;
	} else {
		head_len = MMCBLK_BLOCK_LENGTH - head_len;
	}
	if ((tail_len == 0))
		tail_ptr = NULL;

	if ((head_len >= len) && (tail_addr == offs + head_len - MMCBLK_BLOCK_LENGTH)){
		low_memcpy(buff, tail - head_len + MMCBLK_BLOCK_LENGTH, len);
		return len;
	}

	if ((head_ptr != 0) && (tail_addr == offs + head_len - MMCBLK_BLOCK_LENGTH)) {
		low_memcpy(buff, tail - head_len + MMCBLK_BLOCK_LENGTH, head_len);
		buff += head_len;
		offs += head_len;
		len -= head_len;
		head_len = 0;
		head_ptr = NULL;
	}

	if (head_len >= len) {
		tail_addr = offs + head_len - MMCBLK_BLOCK_LENGTH;
		while (((ret = card->cardOps->read(&(card[bn]), offs + head_len, tail, NULL, NULL, 0)) < 0) && retry-- > 0);
	} else {
		if (tail_ptr != NULL)
			tail_addr = offs + len - tail_len;
		while (((ret = card->cardOps->read(&(card[bn]), offs + head_len, head_ptr, buff + head_len, tail_ptr, len - head_len - tail_len)) < 0) && retry-- > 0);
	}
	if (ret < 0) {
		if (tail_ptr != NULL)
			tail_addr = -1;
		return ret;
	}

	if (head_len >= len) {
		low_memcpy(buff, tail - head_len + MMCBLK_BLOCK_LENGTH, len);
	} else {
		if (head_len != 0) {
			low_memcpy(buff, head - head_len + MMCBLK_BLOCK_LENGTH, head_len);
		}
		if (tail_len != 0) {
			low_memcpy(buff + len - tail_len, tail, tail_len);
		}
	}

	return ret;
}

s32 mmcblk_open(u16 bn, char *name, u32 flags)
{
	s32 ret;

	if ((ret = mmcblk_cardInit(&card[bn])) < 0) {
		plostd_printf(ATTR_ERROR, "MMCBLK: Error %d in card initialization\n", ret);
		return ret;
	}
	if (ret == 1)
		return bn;
	plostd_printf(ATTR_ERROR, "MMCBLK: No card inserted\n");
	return ERR_ARG;
}


s32 mmcblk_close(u16 bn, s32 handle)
{
	return 0;
}


static void clocks_init(void) {
	/* Initialize SDHC1 */
	CCM->CSCDR2 &= ~CCM_CSCDR2_ESDHC1_EN_MASK;
	/* ungate SDHC1 clock */
	CCM->CCGR7 |= CCM_CCGR7_CG2(0x3);
	/* select 66MHz bus clock as base clock for esdhc1 */
	CCM->CSCMR1 |= (3 << CCM_CSCMR1_ESDHC1_CLK_SEL_SHIFT);

	CCM->CSCDR2 &= ~CCM_CSCDR2_ESDHC1_DIV_MASK;
	CCM->CSCDR2 |= CCM_CSCDR2_ESDHC1_DIV(1);
	/* enable esdhc1 base clock */
	CCM->CSCDR2 |= CCM_CSCDR2_ESDHC1_EN_MASK;

	//From Vybrid Reference Manual:
	//-------------------------------------------------------------------------------------------------------
	//Module Name | Signal Name | Direction | Speed | HYS | ODE | PUE | SRE | DSE | PUS | PKE | IBE | MAPPING
	//-------------------------------------------------------------------------------------------------------
	//SDHC        | DAT0        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA26
	//SDHC        | DAT1        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA27
	//SDHC        | DAT2        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA28
	//SDHC        | DAT3        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA29
	//-------------------------------------------------------------------------------------------------------
	//SDHC        | DAT4        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   |
	//SDHC        | DAT5        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   |
	//SDHC        | DAT6        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   |
	//SDHC        | DAT7        | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   |
	//-------------------------------------------------------------------------------------------------------
	//SDHC        | CMD         | I/O       | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA25
	//SDHC        | CLK         | O         | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   | PTA24
	//-------------------------------------------------------------------------------------------------------
	//SDHC        | WP          | I         | 3     | 0   | 0   | 1   | 0   | 7   | 2   | 1   | 1   |
	//-------------------------------------------------------------------------------------------------------
	//GPIO        | CD_SW       | I         | 0     | 0   | 0   | 0   | 0   | 7   | 0   | 0   | 0   | PTA7
	//-------------------------------------------------------------------------------------------------------


	/*Although SDHC_DCLK is output-only signal, ibe needs to be enabled as the module
	requires output clock to be looped-back through the pad’s input buffer to ease timing
	requirements.*/
#if MMCBLK_ENABLED
	u32 val =  (IOMUXC_RGPIO_MUX_MODE(5))		|	/* SDHC mode*/
				(IOMUXC_RGPIO_DSE(7))			|	/* 20 ohm driver*/
				(IOMUXC_RGPIO_SPEED(3)	)		| 	/* 200 MHz */
				(0<<IOMUXC_RGPIO_ODE_SHIFT) 	|	/* CMOS output*/
				(0<<IOMUXC_RGPIO_HYS_SHIFT)		|  	/* CMOS input*/
				(IOMUXC_RGPIO_PUS(2))			|	/* 100 kOhm pull up*/
				(1<<IOMUXC_RGPIO_PKE_SHIFT)		|	/* pull/keeper enabled*/
				(1<<IOMUXC_RGPIO_PUE_SHIFT) 	|	/* pull enable*/
				(1<<IOMUXC_RGPIO_OBE_SHIFT) 	| 	/* enable output (so stays in Hi-Z)*/
				(1<<IOMUXC_RGPIO_IBE_SHIFT);		/* enable input*/
	IOMUXC->RGPIO[14] = val;
	IOMUXC->RGPIO[15] = val;
	IOMUXC->RGPIO[16] = val;
	IOMUXC->RGPIO[17] = val;
	IOMUXC->RGPIO[18] = val;
	IOMUXC->RGPIO[19] = val;
#endif
}


void mmcblk_init() {
	int i = 0;
	clocks_init();

	low_memset(card, 0x0, sizeof(card));
#if MMCBLK_ENABLED
	/* card detect */
	u32 val = IOMUXC_PTA7_MUX_MODE(0)
			| IOMUXC_PTA7_IBE_MASK
			| IOMUXC_PTA7_PKE_MASK
			| IOMUXC_PTA7_PUE_MASK
			| IOMUXC_PTA17_SPEED(1)
			| IOMUXC_PTA7_PUS(2);
	for(i=0; i < MMCBL_PORTS_COUNT; ++i)
	{

		card[i].port  = &mmcblkPorts[i];

		cardDeInit(&card[i]);

		/* initialize controller */
		card[i].port->ioOps.init(&card[i]);
		/* initialize card detect */
		IOMUXC->RGPIO[card[i].port->CDPort] = val;
	}
#endif

}
