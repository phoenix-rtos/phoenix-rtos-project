#ifndef _DEV_MMCBLK_H
#define _DEV_MMCBLK_H

#include "../types.h"
#include "../plostd.h"
#include "../config.h"

#define assert(x) ({0;})
#ifdef DEBUG_MMCBLK
	#define LOG(text, ...) plostd_printf(ATTR_DEBUG, "MMCBLK: " text "\n", ##__VA_ARGS__)
#else
	#define LOG(text, ...) ({0;})
#endif

/**
 * MMCBLK bus widths
 */
typedef enum {
	eMmcblkBusWidth1b=0,
	eMmcblkBusWidth4b,
	eMmcblkBusWidth8b,/** only for MMC, not supported on tower - DB4-7 not connected*/
} MmcblkBusWidth_t;


/**
 * MMCBLK card speed modes
 */
typedef enum
{
	eBasicSpeed=0, /* up to 400 kHz */
	eSDFullSpeed,  /* up to 25 MHz */
	eSDHighSpeed,  /* up to 50 MHz - not supported with bus clock lower than 100M*/
	eMMCFullSpeed, /* up to 20 MHz */
	eMMCHighSpeed  /* up to 52 MHz - not supported with bus clock lower than 104M*/
} MmcblkSpeed_t;

/**
 * MMCBLK card labels
 */
typedef enum {
	eCardUnsupported=-1,
	eCardUnknown=0,
	eCardSD,
	eCardSDIO,
	eCardSDCombo,
	eCardMMC,
	eCardCEATA,
	eCardTypeNum
} MmcblkCardType_t;


typedef enum {
	eLittleEndian=0,
	eBigEndian=1,
	eBigEndianHalfWords=2
} MmcblkEndian_t;

#define MMCBLK_MAX_BASIC_BAUDRATE 400000UL
#define MMCBLK_SD_MAX_FULLSPEED_BAUDRATE 25000000UL
#define MMCBLK_SD_MAX_HIGHSPEED_BAUDRATE 50000000UL
#define MMCBLK_MMC_MAX_FULLSPEED_BAUDRATE 20000000UL
#define MMCBLK_MMC_MAX_HIGHSPEED_BAUDRATE 52000000UL



#define MMCBLK_CMD_TIMEOUT_US                  40000U
#define MMCBLK_CMD12_TIMEOUT_US                500000U
#define MMCBLK_TRANSFER_TIMEOUT_US             750000U



#define MMCBLK_MAX_BLOCKS_PER_TRANSFER 0xFFFFU
#define MMCBLK_MAX_TRANSFERED_BLOCK_SIZE 0x1000U

#define MMCBLK_COMM_TYPE_MASK 0xC000

#define MMCBLK_COMM_BC        0x0000
#define MMCBLK_COMM_BCR       0x4000
#define MMCBLK_COMM_AC        0x8000
#define MMCBLK_COMM_ADTC      0xC000

#define MMCBLK_COMM_GROUP_MASK 0x2000
#define MMCBLK_COMM_CMD 0x0000
#define MMCBLK_COMM_ACMD 0x2000

#define MMCBLK_COMM_RESERVED_MASK 0x1000
#define MMCBLK_COMM_RESERVED 0x1000

#define MMCBLK_COMM_RESPONSE_TYPE_MASK   0x0F00
#define MMCBLK_COMM_RESPONSE_NO_RESPONSE 0x0000
#define MMCBLK_COMM_RESPONSE_R1          0x0100
#define MMCBLK_COMM_RESPONSE_R1b         0x0200
#define MMCBLK_COMM_RESPONSE_R2          0x0300
#define MMCBLK_COMM_RESPONSE_R3          0x0400
#define MMCBLK_COMM_RESPONSE_R4          0x0500
#define MMCBLK_COMM_RESPONSE_R5          0x0600
#define MMCBLK_COMM_RESPONSE_R5b         0x0700
#define MMCBLK_COMM_RESPONSE_R6          0x0800
#define MMCBLK_COMM_RESPONSE_R7          0x0900
#define MMCBLK_COMM_RESPONSE_UNKNOWN     0x0A00

#define MMCBLK_COMM_DAT_OPERATION_MASK 0x0080
#define MMCBLK_COMM_DAT_READ  0x0000
#define MMCBLK_COMM_DAT_WRITE 0x0080

#define MMCBLK_BLOCK_LENGTH 512

#define MMCBLK_COMM_INDEX_MASK 0x003F

/** Resets all MMC and MMCBLK memory cards to idle state */
#define MMCBLK_COMM_GO_IDLE_STATE           (MMCBLK_COMM_BC   | 0 | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_WRITE)

/** Asks all MMC and MMCBLK memory cards in idle state to send their
 * operation conditions register contents in the response on the CMD line */
#define MMCBLK_COMM_SEND_OP_COND            (MMCBLK_COMM_BCR  | 1 | MMCBLK_COMM_RESPONSE_R3 | MMCBLK_COMM_DAT_READ)

/** Asks all cards to send their CID numbers on the CMD line */
#define MMCBLK_COMM_ALL_SEND_CID            (MMCBLK_COMM_BCR  | 2 | MMCBLK_COMM_RESPONSE_R2 | MMCBLK_COMM_DAT_READ)

/** Assigns relative address to the card
 * @warning CMD3 differs for MMC and MMCBLK cards.
 * @warning For MMC cards, it is referred to as SET_RELATIVE_ADDR, with a response type of R1.
 * @warning For MMCBLK cards, it is referred to as SEND_RELATIVE_ADDR, with a response type of R6 (with RCA inside).
 */
#define MMCBLK_COMM_SEND_RELATIVE_ADDR      (MMCBLK_COMM_BCR  | 3 | MMCBLK_COMM_RESPONSE_R6 | MMCBLK_COMM_DAT_WRITE)

#define MMCBLK_COMM_SET_RELATIVE_ADDR       (MMCBLK_COMM_BCR  | 3 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Programs the DSR of all cards. */
#define MMCBLK_COMM_SET_DSR                 (MMCBLK_COMM_BC   | 4 | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_WRITE)


/** Asks all MMCBLKIO cards in idle state to send their operation conditions
 * register contents in the response on the CMD line */
#define MMCBLK_COMM_IO_SEND_OP_COND         (MMCBLK_COMM_BC   | 5 | MMCBLK_COMM_RESPONSE_R4 | MMCBLK_COMM_DAT_READ)


/** Checks Switch ability (mode 0) and switch card function (mode 1)
 * @warning THIS COMMAND APPLIES ONLY FOR HIGH SPEED MMCBLK CARDS
*/
#define MMCBLK_COMM_SWITCH_FUNC             (MMCBLK_COMM_ADTC | 6 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)
/** Switches the mode of operation of selected card
 *  or modifies the EXT_CMMCBLK registers
 * @warning THIS COMMAND APPLIES ONLY FOR HIGH SPEED MMC CARDS
 */
#define MMCBLK_COMM_SWITCH                  (MMCBLK_COMM_AC   | 6 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** Toggles a card between the stand-by and transfer states
or between the programming and disconnect states. In both
cases, the card is selected by its own relative address and
gets deselected by any other address. Address 0 deselects all */
#define MMCBLK_COMM_SELECT_DESELECT_CARD    (MMCBLK_COMM_AC   | 7 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** The card sends its EXT_CMMCBLK register as a block of data, with a block size of 512 bytes */
#define MMCBLK_COMM_SEND_EXT_CMMCBLK            (MMCBLK_COMM_ADTC | 8 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

#define MMCBLK_COMM_SEND_IF_COND            (MMCBLK_COMM_BCR | 8 | MMCBLK_COMM_RESPONSE_R7 | MMCBLK_COMM_DAT_READ)

/** Addressed card sends its card-specific data (CMMCBLK) on the CMD line */
#define MMCBLK_COMM_SEND_CSD                (MMCBLK_COMM_AC   | 9 | MMCBLK_COMM_RESPONSE_R2 | MMCBLK_COMM_DAT_READ)

/** Addressed card sends its card-identification (CID) on the CMD line */
#define MMCBLK_COMM_SEND_CID                (MMCBLK_COMM_AC   | 10 | MMCBLK_COMM_RESPONSE_R2 | MMCBLK_COMM_DAT_READ)

/** Reads data stream from the card, starting at the given
address, until a STOP_TRANSMISSION follows */
#define MMCBLK_COMM_READ_DAT_UNTIL_STOP     (MMCBLK_COMM_ADTC | 11 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Forces card to stop transmission */
#define MMCBLK_COMM_STOP_TRANSMISSION       (MMCBLK_COMM_AC   | 12 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** Addressed card sends its status register */
#define MMCBLK_COMM_SEND_STATUS             (MMCBLK_COMM_AC   | 13 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

#define MMCBLK_COMM_CMD14_RESERVED          (MMCBLK_COMM_BC   | 14 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_WRITE)

/** Sets card to inactive state to protect card stack against communication breakdowns. */
#define MMCBLK_COMM_GO_INACTIVE_STATE       (MMCBLK_COMM_AC   | 15 | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Sets the block length (in bytes) for all following block
commands (read and write). Default block length is specified in the CMMCBLK */
#define MMCBLK_COMM_SET_BLOCKLEN            (MMCBLK_COMM_AC   | 16 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Reads a block of the size selected by the SET_BLOCKLEN command */
#define MMCBLK_COMM_READ_SINGLE_BLOCK       (MMCBLK_COMM_ADTC | 17 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Continuously transfers data blocks from card to host until
interrupted by a stop command */
#define MMCBLK_COMM_READ_MULTIPLE_BLOCK     (MMCBLK_COMM_ADTC | 18 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

#define MMCBLK_COMM_CMD19_RESERVED          (MMCBLK_COMM_BC   | 19 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Writes data stream from the host, starting at the given
address, until a STOP_TRANSMISION follow */
#define MMCBLK_COMM_WRITE_DAT_UNTIL_STOP    (MMCBLK_COMM_ADTC | 20 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

#define MMCBLK_COMM_CMD21_RESERVED          (MMCBLK_COMM_BC   | 21 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD22_RESERVED          (MMCBLK_COMM_BC   | 22 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

#define MMCBLK_COMM_SET_BLOCK_COUNT         (MMCBLK_COMM_AC   | 23 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Writes a block of the size selected by the SET_BLOCKLEN command */
#define MMCBLK_COMM_WRITE_BLOCK             (MMCBLK_COMM_ADTC | 24 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Continuously writes blocks of data until a STOP_TRANSMISSION follows */
#define MMCBLK_COMM_WRITE_MULTIPLE_BLOCK    (MMCBLK_COMM_ADTC | 25 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Programming of the card identification register. This
command shall be issued only once per card. The card
contains hardware to prevent this operation after the first
programming. Normally this command is reserved for the manufacturer */
#define MMCBLK_COMM_PROGRAM_CID             (MMCBLK_COMM_ADTC | 26 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Programming of the programmable bits of the CMMCBLK */
#define MMCBLK_COMM_PROGRAM_CSD             (MMCBLK_COMM_ADTC | 27 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** If the card has write protection features, this command sets
the write protection bit of the addressed group. The
properties of write protection are coded in the card specific data (WP_GRP_SIZE) */
#define MMCBLK_COMM_SET_WRITE_PROT          (MMCBLK_COMM_AC   | 28 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** If the card provides write protection features, this
command clears the write protection bit of the addressed group */
#define MMCBLK_COMM_CLR_WRITE_PROT          (MMCBLK_COMM_AC   | 29 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** If the card provides write protection features, this
command asks the card to send the status of the write protection bits */
#define MMCBLK_COMM_SEND_WRITE_PROT         (MMCBLK_COMM_ADTC | 30 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

#define MMCBLK_COMM_CMD31_RESERVED          (MMCBLK_COMM_BC   | 31 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_DAT_READ)

/** Sets the address of the first sector of the erase group */
#define MMCBLK_COMM_TAG_SECTOR_START        (MMCBLK_COMM_AC   | 32 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Sets the address of the last sector in a continuous range
within the selection of a single sector to be selected for erase */
#define MMCBLK_COMM_TAG_SECTOR_END          (MMCBLK_COMM_AC   | 33 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Removes one previously selected sector from the erase selection */
#define MMCBLK_COMM_UNTAG_SECTOR            (MMCBLK_COMM_AC   | 34 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Sets the address of the first erase group within a range to be selected for erase */
#define MMCBLK_COMM_TAG_ERASE_GROUP_START   (MMCBLK_COMM_AC   | 35 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Sets the address of the last erase group within a continuous range to be selected for erase */
#define MMCBLK_COMM_TAG_ERASE_GROUP_END     (MMCBLK_COMM_AC   | 36 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Removes one previously selected erase group from the erase selection */
#define MMCBLK_COMM_UNTAG_ERASE_GROUP       (MMCBLK_COMM_AC   | 37 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Erase all previously selected sectors */
#define MMCBLK_COMM_ERASE                   (MMCBLK_COMM_AC   | 38 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

/** Used to write and read 8-bit (register) data fields.
 * The command addresses a card, and a register, and provides
 * the data for writing if the write flag is set. The R4 response
 * contains data read from the address register. This
 * command accesses application dependent registers which are not
 * defined in the MMC standard */
#define MMCBLK_COMM_FAST_IO                 (MMCBLK_COMM_AC   | 39 | MMCBLK_COMM_RESPONSE_R4 | MMCBLK_COMM_DAT_WRITE)

/** Sets the system into interrupt mode */
#define MMCBLK_COMM_GO_IRQ_STATE            (MMCBLK_COMM_BCR  | 40 | MMCBLK_COMM_RESPONSE_R5 | MMCBLK_COMM_DAT_WRITE)

#define MMCBLK_COMM_CMD41_RESERVED          (MMCBLK_COMM_BC   | 41 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Used to set/reset the password or lock/unlock the
card. The size of the data block is set by the SET_BLOCK_LEN command */
#define MMCBLK_COMM_LOCK_UNLOCK             (MMCBLK_COMM_ADTC | 42 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)

#define MMCBLK_COMM_CMD43_RESERVED          (MMCBLK_COMM_BC   | 43 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD44_RESERVED          (MMCBLK_COMM_BC   | 44 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD45_RESERVED          (MMCBLK_COMM_BC   | 45 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD46_RESERVED          (MMCBLK_COMM_BC   | 46 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD47_RESERVED          (MMCBLK_COMM_BC   | 47 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD48_RESERVED          (MMCBLK_COMM_BC   | 48 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD49_RESERVED          (MMCBLK_COMM_BC   | 49 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD50_RESERVED          (MMCBLK_COMM_BC   | 50 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD51_RESERVED          (MMCBLK_COMM_BC   | 51 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Access a single register within the total 128k of register space in any I/O function */
#define MMCBLK_COMM_IO_RW_DIRECT            (MMCBLK_COMM_AC   | 52 | MMCBLK_COMM_RESPONSE_R5 | MMCBLK_COMM_DAT_WRITE)//readwrite? - MMCBLKIO

/** Accesses a multiple I/O register with a single command.
 * Allows the reading or writing of a large number of I/O registers */
#define MMCBLK_COMM_IO_RW_EXTENDED          (MMCBLK_COMM_AC   | 53 | MMCBLK_COMM_RESPONSE_R5 | MMCBLK_COMM_DAT_WRITE)//readwrite? - MMCBLKIO

#define MMCBLK_COMM_CMD54_RESERVED          (MMCBLK_COMM_BC   | 54 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Indicates to the card that the next command is an
application specific command rather that a standard command */
#define MMCBLK_COMM_APP_CMD                 (MMCBLK_COMM_AC   | 55 | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_WRITE)

/** Used either to transfer a data block to the card or to get a
data block from the card for general purpose / application
specific commands. The size of the data block is set by the SET_BLOCK_LEN command */
#define MMCBLK_COMM_GEN_CMD                 (MMCBLK_COMM_ADTC | 56 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE)//readwrite? - general purpose/application specific commands

#define MMCBLK_COMM_CMD57_RESERVED          (MMCBLK_COMM_BC   | 57 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD58_RESERVED          (MMCBLK_COMM_BC   | 58 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD59_RESERVED          (MMCBLK_COMM_BC   | 59 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** These registers are used to control the behavior of the
device and to retrieve status information regarding the
operation of the device. All Status and Control registers
are WORD (32-bit) in size and are WORD aligned. CMD60
shall be used to read and write these registers */
#define MMCBLK_COMM_RW_MULTIPLE_REGISTER    (MMCBLK_COMM_ADTC | 60 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE) //readwrite?

/** The host issues a RW_MULTIPLE_BLOCK (CMD61) to begin the data transfer. */
#define MMCBLK_COMM_RW_MULTIPLE_BLOCK       (MMCBLK_COMM_ADTC | 61 | MMCBLK_COMM_RESPONSE_R1b | MMCBLK_COMM_DAT_WRITE) //readwrite?

#define MMCBLK_COMM_CMD62_RESERVED          (MMCBLK_COMM_BC   | 62 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)
#define MMCBLK_COMM_CMD63_RESERVED          (MMCBLK_COMM_BC   | 63 | MMCBLK_COMM_RESERVED | MMCBLK_COMM_RESPONSE_NO_RESPONSE | MMCBLK_COMM_DAT_READ)

/** Defines the data bus width ('00'=61bit or '10'=4bit bus) to
 * be used for data transfer. The allowed data bus widths
 * are given in SCR register
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
*/
#define MMCBLK_COMM_SET_BUS_WIDTH           (MMCBLK_COMM_AC   | 6  | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Send the MMCBLK memory card status.
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SD_STATUS               (MMCBLK_COMM_ADTC | 13 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Send the number of the written sectors (without errors).
 * Responds with 32-bit plus the CRC data block.
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SEND_NUM_WR_SECTORS     (MMCBLK_COMM_ADTC | 22 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/**
 * @warning - datasheet says nothing about it
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SET_WE_BLK_ERASE_COUNT  (MMCBLK_COMM_AC   | 23 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Asks the accessed card to send its operating condition
 *  register (OCR) contents in the response on the CMD line
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SD_APP_OP_COND          (MMCBLK_COMM_BCR  | 41 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R3 | MMCBLK_COMM_DAT_READ)

/**
 * @warning - datasheet says nothing about it
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SET_CLR_CARD_DETECT     (MMCBLK_COMM_AC   | 42 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/** Reads the MMCBLK Configuration Register (SCR)
 * @warning - this is ACMD - it shall be preceeded with MMCBLK_COMM_APP_CMD command.
 */
#define MMCBLK_COMM_SEND_SCR                (MMCBLK_COMM_ADTC | 51 | MMCBLK_COMM_ACMD | MMCBLK_COMM_RESPONSE_R1 | MMCBLK_COMM_DAT_READ)

/* Registers taken from http://www.kosmodrom.com.ua/pdf/RASPBERRY-PI-PROG-4GB-MMCBLKCARD.pdf
	MMBTFxxGUBCA-xMExx
	Samsung MMCBLK & MicroMMCBLK Card product family
	MMCBLKA 3.0 specification compliant-Up to High Speed mode */
typedef u32 MmcblkOCRReg_t;

#define MMCBLK_OCR_V20_V21    (1 << 8)
#define MMCBLK_OCR_V21_V22    (1 << 9)
#define MMCBLK_OCR_V22_V23    (1 << 10)
#define MMCBLK_OCR_V23_V24    (1 << 11)
#define MMCBLK_OCR_V24_V25    (1 << 12)
#define MMCBLK_OCR_V25_V26    (1 << 13)
#define MMCBLK_OCR_V26_V27    (1 << 14)
#define MMCBLK_OCR_V27_V28    (1 << 15)
#define MMCBLK_OCR_V28_V29    (1 << 16)
#define MMCBLK_OCR_V29_V30    (1 << 17)
#define MMCBLK_OCR_V30_V31    (1 << 18)
#define MMCBLK_OCR_V31_V32    (1 << 19)
#define MMCBLK_OCR_V32_V33    (1 << 20)
#define MMCBLK_OCR_V33_V34    (1 << 21)
#define MMCBLK_OCR_V34_V35    (1 << 22)
#define MMCBLK_OCR_V35_V36    (1 << 23)
#define MMCBLK_OCR_S18A       (1 << 24)       // only UHS-1 card supports this
#define MMCBLK_OCR_UHS_CARD_STATUS  (1 << 29)
#define MMCBLK_OCR_CCS        (1 << 30)       // Card capacity status - valid only when CPUPS is set
#define MMCBLK_OCR_CPUPS      (1 << 31)       // Card power up status - low if card still in power up routine



typedef struct __attribute__(( packed )){
	u32 cid[4];
} MmcblkCIDReg_t;


#define MMCBLK_CID_GET_MID(cid_reg)       (((cid_reg).cid[0] >> 16) & 0xFFU)    /* Manufacturer ID       BINARY */
#define MMCBLK_CID_GET_OID(cid_reg)       ((cid_reg).cid[0] & 0xFFFFU)          /* OEM/Application ID    ASCII  */
#define MMCBLK_CID_GET_PNM(cid_reg)       (((u64) 0L) | (((u64)((cid_reg).cid[1])) << 8) | (((u64)((cid_reg).cid[2]) >> 24) & 0xFFU))/* Product name          ASCII  */
#define MMCBLK_CID_GET_PRV(cid_reg)       (((cid_reg).cid[2] >> 16) & 0xFF)    /* Product revision      BCD    */
#define MMCBLK_CID_GET_PSN(cid_reg)       ((((cid_reg).cid[2] << 16) & 0xFFFF0000U) | (((cid_reg).cid[3] >> 16) & 0xFFFFU))   /* Product serial number BINARY */
#define MMCBLK_CID_GET_MDT(cid_reg)       ((cid_reg).cid[3] & 0xFFFU)    /* Manufacturing date    BCD    */


/* TAAC bits (taken from http://www.mikrocontroller.net/attachment/21920/MMCBLKHC_MMCBLKM04G7B7_08G7B7.pdf):
 * 2:0 0 1ns; 1 10ns; 2 100ns; 3 1us; 4 10us; 5 100us; 6 1ms; 7 10ms
 * 6:3 0 reserved; 1 1.0; 2 1.2; 3 1.3; 4 1.5; 5 2.0; 6 2.5; 7 3.0; 8 3.5; 9 4.0; A 4.5; B 5.0;
 *     C 5.5; D 6.0; E 7.0; F 8.0
 */
typedef union __attribute__(( packed )){
	u32 csd[4];
} MmcblkCSD10Reg_t;


#define MMCBLK_CSD10_GET_FILE_FORMAT(csd_reg) (((csd).csd[3] >> 2) & 0x3U)
#define MMCBLK_CSD10_GET_TMP_WRITE_PROTECT(csd_reg)   (((csd_reg).csd[3] >> 4) & 0x1U)
#define MMCBLK_CSD10_GET_PERM_WRITE_PROTECT(csd_reg)  (((csd_reg).csd[3] >> 5) & 0x1U)
#define MMCBLK_CSD10_GET_COPY(csd_reg)                (((csd_reg).csd[3] >> 6) & 0x1U) /* COPY FLAG (OTP) */
#define MMCBLK_CSD10_GET_FILE_FORMAT_GRP(csd_reg)     (((csd_reg).csd[3] >> 7) & 0x1U)

#define MMCBLK_CSD10_GET_WRITE_BL_PARTIAL(csd_reg)    (((csd_reg).csd[3] >> 13) & 0x1U) /* Partial blocks for write allowed */
#define MMCBLK_CSD10_GET_WRITE_BL_LEN(csd_reg)        (((csd_reg).csd[3] >> 14) & 0xFU) /* Max write data block length */
#define MMCBLK_CSD10_GET_R2W_FACTOR(csd_reg)          (((csd_reg).csd[3] >> 18) & 0x7U) /* Write speed factor */
#define MMCBLK_CSD10_GET_WP_GRP_ENABLE(csd_reg)       (((csd_reg).csd[3] >> 23) & 0x1U) /* Write protect group enable */
#define MMCBLK_CSD10_GET_WP_GRP_SIZE(csd_reg)         (((csd_reg).csd[3] >> 24) & 0x3FU) /* Write protect group size */

#define MMCBLK_CSD10_GET_SECTOR_SIZE(csd_reg)         ((((csd_reg).csd[2] << 1) & 0x7EU) | (((csd_reg).csd[3] >> 31) & 0x1U)) /* Erase sector size */

#define MMCBLK_CSD10_GET_ERASE_BLK_EN(csd_reg)        (((csd_reg).csd[2] >> 6) & 0x1U) /* Erase single block enable */
#define MMCBLK_CSD10_GET_C_SIZE_MULT(csd_reg)         (((csd_reg).csd[2] >> 7) & 0x7U) /* Device size multipier */
#define MMCBLK_CSD10_GET_VDD_W_CURR_MAX(csd_reg)      (((csd_reg).csd[2] >> 10) & 0x7U) /* Max write current @ VDDmax */
#define MMCBLK_CSD10_GET_VDD_W_CURR_MIN(csd_reg)      (((csd_reg).csd[2] >> 13) & 0x7U) /* Max write current @ VDDmin */
#define MMCBLK_CSD10_GET_VDD_R_CURR_MAX(csd_reg)      (((csd_reg).csd[2] >> 16) & 0x7U) /* Max read current @ VDDmax */
#define MMCBLK_CSD10_GET_VDD_R_CURR_MIN(csd_reg)      (((csd_reg).csd[2] >> 19) & 0x7U) /* Max read current @ VDDmin */

#define MMCBLK_CSD10_GET_C_SIZE(csd_reg)              ((((csd_reg).csd[1] << 10) & 0xC00U) | (((csd_reg).csd[2] >> 22) & 0x3FFU))/* Device size */

#define MMCBLK_CSD10_GET_DSR_IMP(csd_reg)             (((csd_reg).csd[1] >> 4) & 0x1U) /* DSR implemented */
#define MMCBLK_CSD10_GET_READ_BLK_MISALIGN(csd_reg)   (((csd_reg).csd[1] >> 5) & 0x1U) /* Read block misalignment */
#define MMCBLK_CSD10_GET_WRITE_BLK_MISALIGN(csd_reg)  (((csd_reg).csd[1] >> 6) & 0x1U) /* Write block misalignment */
#define MMCBLK_CSD10_GET_READ_BL_PARTIAL(csd_reg)     (((csd_reg).csd[1] >> 7) & 0x1U) /* Partial blocks for read allowed */
#define MMCBLK_CSD10_GET_READ_BL_LEN(csd_reg)         (((csd_reg).csd[1] >> 8) & 0xFU) /* Max read data block length */
#define MMCBLK_CSD10_GET_CCC(csd_reg)                 (((csd_reg).csd[1] >> 12) & 0xFFFU)/* Card Command Classes */
#define MMCBLK_CSD10_GET_TRAN_SPEED(csd_reg)          (((csd_reg).csd[1] >> 24) & 0xFFU) /* Max data transfer rate */

#define MMCBLK_CSD10_GET_NSAC(csd_reg)                ((csd_reg).csd[0] & 0xFFU)8 /* Data read access-time 2 in CLK cycles (NSAC * 100) */
#define MMCBLK_CSD10_GET_TAAC(csd_reg)                (((csd_reg).csd[0] >> 8) & 0xFFU) /* Data read access-time 1 */

#define MMCBLK_CSD10_GET_CSD_STRUCTURE(csd_reg)       (((csd_reg).csd[0] >> 22) & 0x3U) /* CMMCBLK structure for 1.0 this will be 0*/




typedef struct __attribute__(( packed )){
	u32 csd[4];
} MmcblkCSD20Reg_t;



#define MMCBLK_CSD20_GET_FILE_FORMAT(csd_reg)         (((csd_reg).csd[3] >> 2) & 0x3U)
#define MMCBLK_CSD20_GET_TMP_WRITE_PROTECT(csd_reg)   (((csd_reg).csd[3] >> 4) & 0x1U)
#define MMCBLK_CSD20_GET_PERM_WRITE_PROTECT(csd_reg)  (((csd_reg).csd[3] >> 5) & 0x1U)
#define MMCBLK_CSD20_GET_COPY(csd_reg)                (((csd_reg).csd[3] >> 6) & 0x1U) /* COPY FLAG (OTP) */
#define MMCBLK_CSD20_GET_FILE_FORMAT_GRP(csd_reg)     (((csd_reg).csd[3] >> 7) & 0x1U)

#define MMCBLK_CSD20_GET_WRITE_BL_PARTIAL(csd_reg)    (((csd_reg).csd[3] >> 13) & 0x1U) /* Partial blocks for write allowed */
#define MMCBLK_CSD20_GET_WRITE_BL_LEN(csd_reg)        (((csd_reg).csd[3] >> 14) & 0xFU) /* Max write data block length */

#define MMCBLK_CSD20_GET_R2W_FACTOR(csd_reg)          (((csd_reg).csd[3] >> 18) & 0x7U) /* Write speed factor */
#define MMCBLK_CSD20_GET_WP_GRP_ENABLE(csd_reg)       (((csd_reg).csd[3] >> 23) & 0x1U) /* Write protect group enable */
#define MMCBLK_CSD20_GET_WP_GRP_SIZE(csd_reg)         (((csd_reg).csd[3] >> 24) & 0x3FU) /* Write protect group size */
#define MMCBLK_CSD20_GET_SECTOR_SIZE(csd_reg)         ((((csd_reg).csd[2] << 1) & 0x7EU) | (((csd_reg).csd[3] >> 31) & 0x1U)) /* Erase sector size */
#define MMCBLK_CSD20_GET_ERASE_BLK_EN(csd_reg)        (((csd_reg).csd[2] >> 6) & 0x1U) /* Erase single block enable */
#define MMCBLK_CSD20_GET_RESERVED3(csd_reg)           (((csd_reg).csd[2] >> 7) & 0x1U) /* Device size multipier */
#define MMCBLK_CSD20_GET_C_SIZE(csd_reg)              (((csd_reg).csd[2] >> 8) & 0x3FFFFFU) /* Device size */
#define MMCBLK_CSD20_GET_RESERVED4(csd_reg)

#define MMCBLK_CSD20_GET_DSR_IMP(csd_reg)             (((csd_reg).csd[1] >> 4) & 0x1U) /* DSR implemented */
#define MMCBLK_CSD20_GET_READ_BLK_MISALIGN(csd_reg)   (((csd_reg).csd[1] >> 5) & 0x1U) /* Read block misalignment */
#define MMCBLK_CSD20_GET_WRITE_BLK_MISALIGN(csd_reg)  (((csd_reg).csd[1] >> 6) & 0x1U) /* Write block misalignment */
#define MMCBLK_CSD20_GET_READ_BL_PARTIAL(csd_reg)     (((csd_reg).csd[1] >> 7) & 0x1U) /* Partial blocks for read allowed */
#define MMCBLK_CSD20_GET_READ_BL_LEN(csd_reg)         (((csd_reg).csd[1] >> 8) & 0xFU) /* Max read data block length */
#define MMCBLK_CSD20_GET_CCC(csd_reg)                 (((csd_reg).csd[1] >> 12) & 0xFFFU)/* Card Command Classes */
#define MMCBLK_CSD20_GET_TRAN_SPEED(csd_reg)          (((csd_reg).csd[1] >> 24) & 0xFFU) /* Max data transfer rate */

#define MMCBLK_CSD20_GET_NSAC(csd_reg)                ((csd_reg).csd[0] & 0xFFU) /* Data read access-time 2 in CLK cycles (NSAC * 100) */
#define MMCBLK_CSD20_GET_TAAC(csd_reg)                (((csd_reg).csd[0] >> 8) & 0xFFU) /* Data read access-time 1 */
#define MMCBLK_CSD20_GET_CSD_STRUCTURE(csd_reg)       (((csd_reg).csd[0] >> 22) & 0x3U) /* CMMCBLK structure - for 2.0 this will be 1*/

const static u32 mmcblk_br_unit[] = { 10000, 100000, 1000000, 10000000 };
const static u8 mmcblk_br_value[] = { 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };

#define MMCBLK_CSD_GET_BAUDRATE(csd_reg) (mmcblk_br_unit[MMCBLK_CSD10_GET_TRAN_SPEED(csd_reg) & 0x7U] * mmcblk_br_value[(MMCBLK_CSD10_GET_TRAN_SPEED(csd_reg) >> 3) & 0xFU])
#define MMCBLK_CSD_GET_CAPACITY(csd_reg) ((u32)(MMCBLK_CSD20_GET_CSD_STRUCTURE(csd_reg) ? ((MMCBLK_CSD20_GET_C_SIZE(csd_reg) + 1) << 10) : ((MMCBLK_CSD10_GET_C_SIZE(csd_reg) + 1) * (1 << (MMCBLK_CSD10_GET_C_SIZE_MULT(csd_reg) +2)) * (1 << (MMCBLK_CSD10_GET_READ_BL_LEN(csd_reg)-9)))))

typedef union __attribute__(( packed )){
	MmcblkCSD10Reg_t csd10;
	MmcblkCSD20Reg_t csd20;
} MmcblkCSDReg_t;


typedef union __attribute__(( packed )){
	u16 rca;
} MmcblkRCAReg_t;

typedef struct __attribute__(( packed )){
	u32 scr[2];
} MmcblkSCRReg_t;


#define MMCBLK_SCR_GET_CMD_SUPPORT(scr_reg)            ((scr_reg).scr[0] & 0x3U)         /* Command support bits */
#define MMCBLK_SCR_GET_RESERVED1(scr_reg)              (((scr_reg).scr[0] >> 2) & 0x1FFFU)
#define MMCBLK_SCR_GET_SD_SPEC3(scr_reg)               (((scr_reg).scr[0] >> 15) & 0x1U) /* Spec version 3 or higher */
#define MMCBLK_SCR_GET_SD_BUS_WIDTHS(scr_reg)          (((scr_reg).scr[0] >> 16) & 0xFU) /* Dat bus widths supported */
#define MMCBLK_SCR_GET_SD_SECURITY(scr_reg)            (((scr_reg).scr[0] >> 20) & 0x7U) /* MMCBLK Security support */
#define MMCBLK_SCR_GET_DATA_STAT_AFTER_ERASE(scr_reg)  (((scr_reg).scr[0] >> 23) & 0x1U)
#define MMCBLK_SCR_GET_SD_SPEC(scr_reg)                (((scr_reg).scr[0] >> 24) & 0xFU) /* MMCBLK Memory Card spec version */
#define MMCBLK_SCR_GET_SCR_STRUCTURE(scr_reg)          (((scr_reg).scr[0] >> 28) & 0xFU) /* Scr structure */


typedef struct __attribute__(( packed )){
	u32 status[16];
} MmcblkStatusReg_t;

#define MMCBLK_STATUS_GET_ERASE_OFFSET(status_reg)            (((status_reg).status[3] >> 16) & 0x3U)
#define MMCBLK_STATUS_GET_ERASE_TIMEOUT(status_reg)           (((status_reg).status[3] >> 18) & 0x3FU)
#define MMCBLK_STATUS_GET_ERASE_SIZE(status_reg)              (((status_reg).status[2] << 8) & 0xFF00U) | (((status_reg).status[3] >> 24) & 0xFFU)

#define MMCBLK_STATUS_GET_RESERVED13(status_reg)              (((status_reg).status[2] >> 8) & 0xFU)

#define MMCBLK_STATUS_GET_AU_SIZE(status_reg)                 (((status_reg).status[2] >> 12) & 0xFU)
#define MMCBLK_STATUS_GET_PERFORMANCE_MOVE(status_reg)        (((status_reg).status[2] >> 16) & 0xFFU)
#define MMCBLK_STATUS_GET_SPEED_CLASS(status_reg)             (((status_reg).status[2] >> 24) & 0xFFU)
#define MMCBLK_STATUS_GET_SIZE_OF_PROTECTED_AREA(status_reg)  ((status_reg).status[1] & 0xFFFFFFFFU)
#define MMCBLK_STATUS_GET_MMCBLK_CARD_TYPE(status_reg)            ((status_reg).status[0] & 0xFFFFU)

#define MMCBLK_STATUS_GET_RESERVED14(status_reg)              (((status_reg).status[0] >> 16) & 0x3FU)
#define MMCBLK_STATUS_GET_RESERVED15(status_reg)              (((status_reg).status[0] >> 22) & 0x7FU)

#define MMCBLK_STATUS_GET_SECURED_MODE(status_reg)   (((status_reg).status[0] >> 29) & 0x1U)
#define MMCBLK_STATUS_GET_DATA_BUS_WIDTH(status_reg) (((status_reg).status[0] >> 30) & 0x3U)

//normal response
typedef union __attribute__(( packed )){
	u32 response;
	struct {
		unsigned RESERVED0 : 2;
		unsigned RESERVED1 : 1;
		unsigned AKE_SEQ_ERROR : 1;
		unsigned RESERVED2 : 1;//this is used by MMCBLKIO cards
		unsigned APP_CMD : 1;
		unsigned RESERVED3 : 2;
		unsigned READY_FOR_DATA : 1;
		unsigned CURRENT_STATE : 4;
		unsigned ERASE_RESET : 1;
		unsigned CARD_ECC_DISABLED : 1;
		unsigned WP_ERASE_SKIP : 1;
		unsigned CSD_OVERWRITE : 1;
		unsigned RESERVED4 : 2;
		unsigned ERROR : 1;
		unsigned CC_ERROR : 1;
		unsigned CARD_ECC_FAILED : 1;
		unsigned ILLEGAL_COMMAND : 1;
		unsigned COM_CRC_ERROR : 1;
		unsigned LOCK_UNLOCK_FAILED : 1;
		unsigned CARD_IS_LOCKED : 1;
		unsigned WP_VIOLATION : 1;
		unsigned ERASE_PARAM : 1;
		unsigned ERASE_SEQ_ERROR : 1;
		unsigned BLOCK_LEN_ERROR : 1;
		unsigned ADDRESS_ERROR : 1;
		unsigned OUT_OF_RANGE : 1;
	} bits;
} MmcblkR1Response_t;


//CID/CSD register
typedef union {
	MmcblkCIDReg_t cid;
	MmcblkCSDReg_t csd;
} MmcblkR2Response_t;

//ocr register
typedef union {
	MmcblkOCRReg_t ocr;
} MmcblkR3Response_t;

//ocr register for MMCBLKIO
typedef MmcblkR3Response_t MmcblkR4Response_t;

//MMCBLKIO response
typedef MmcblkR1Response_t MmcblkR5Response_t;

typedef union __attribute__(( packed )){
	u32 response;
	struct {
		unsigned RESERVED0 : 2;
		unsigned RESERVED1 : 1;
		unsigned AKE_SEQ_ERROR : 1;
		unsigned RESERVED2 : 1;
		unsigned APP_CMD : 1;
		unsigned RESERVED3 : 2;
		unsigned READY_FOR_DATA : 1;
		unsigned CURRENT_STATE : 4;
		unsigned ERROR : 1;
		unsigned ILLEGAL_COMMAND : 1;
		unsigned COM_CRC_ERROR : 1;
		unsigned RCA : 16;
	} bits;
} MmcblkR6Response_t;

typedef union __attribute__(( packed )){
	u32 response;
	struct{
		u32 PATTERN_ECHO : 8;
		u32 VOLTAGE_ACCEPTED : 4;
		u32 RESERVED : 20;
	} bits;
}MmcblkR7Response_t;

typedef struct __attribute__(( packed )){
	int responseType;
	union {
		MmcblkR1Response_t r1;
		MmcblkR1Response_t r1b;
		MmcblkR2Response_t r2;
		MmcblkR3Response_t r3;
		MmcblkR4Response_t r4;
		MmcblkR5Response_t r5;
		MmcblkR5Response_t r5b;
		MmcblkR6Response_t r6;
		MmcblkR7Response_t r7;
	}response;
	int timeout;
	int error;
	int busy;
} MmcblkResponse_t;

#define MMCBLK_VOLTAGE_ACCEPTED_2_7__3_6 0x1
#define MMCBLK_VOLTAGE_ACCEPTED_LOW_VOLTAGE 0x2


#define MMCBLK_EVENT_TRANSFER_COMPLETED 0x1U
#define MMCBLK_EVENT_TRANSFER_ERROR     0x2U

#define MMCBLK_EVENT_COMMAND_COMPLETED  0x4U
#define MMCBLK_EVENT_COMMAND_ERROR      0x8U

#define MMCBLK_EVENT_COMMAND_TIMEOUT    0x10U
#define MMCBLK_EVENT_TRANSFER_TIMEOUT   0x20U
#define MMCBLK_EVENT_CARD_REMOVED       0x40U


typedef struct FreePtr{
	void *freeptr;
	struct FreePtr *next;
} FreePtr;

typedef struct MmcblkIoOps_t{
	int (*init)(void *card);
	int (*reset)(void *card);
	int (*sendCommand)(void *card, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size);
	int (*sendCommandWithTransfer)(void *card, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size, void *headbuff, void *bodybuff, void *tailbuff, u32 bufsize);
	void (*waitForResponse)(void *card, int cmd, MmcblkResponse_t *ret);
	int (*transferWait)(void *card, u32 bytes);
	int (*waitBusy)(void *card, u32 bytes);
	int (*switchHighSpeed)(void *card, u32 baudrate);
	int (*setupBaudRate)(void *card, u32 baudrate);
	int (*setupBusWidth)(void *cardPtr, MmcblkBusWidth_t width);
	int (*setupEndian)(void *cardPtr, MmcblkEndian_t width);
} MmcblkIoOps_t;

struct MmcblkCard_t;
typedef struct MmcblkCard_t MmcblkCard_t;

typedef struct MmcblkCardOps_t{

	int (*init)(MmcblkCard_t *card);
	void (*deinit)(MmcblkCard_t *card);
// 	int (*inserted)(MmcblkCard_t *card);
// 	int (*switchHighSpeed)(MmcblkCard_t *card);
// 
// 	int (*write)(MmcblkCard_t *card, unsigned int offs, char *buff, unsigned int len);
	int (*read)(MmcblkCard_t *card, unsigned int offs, char *headbuff, char *bodybuff, char *tailbuff, unsigned int len);

} MmcblkCardOps_t;


typedef struct MmcblkPortDesc_t{
	void * ioBase;
	int irq;
	MmcblkIoOps_t ioOps;
	int CSPort;
	int CDPort;
	/* CDActive == 0 means low state indicates card inserted
	   CDActive == 1 means high state indicates card inserted
	   CDActive == 2 means ignore card detect state
	 */
	int CDActive;
} MmcblkPortDesc_t;


/** sd card descriptor */
struct MmcblkCard_t {
	MmcblkCardOps_t *cardOps;
	MmcblkPortDesc_t *port;
	void *ioBase;

	MmcblkCardType_t label;
	MmcblkBusWidth_t busWidth;
	MmcblkSpeed_t    speed;
	u32 sectorSize;
	u32 baudRate;
	u32 capacity;//expressed in sectors
	unsigned voltage;

	MmcblkCSDReg_t CSD;
	MmcblkCIDReg_t CID;
	MmcblkSCRReg_t SCR;
	u32 RCA;
	u32 OCR;

	u32 eventReg;
};


void mmcblk_init(void);
s32 mmcblk_open(u16 bn, char *name, u32 flags);
s32 mmcblk_read(u16 bn, s32 handle, u64 *offs, u8 *buff, u32 len);
s32 mmcblk_close(u16 bn, s32 handle);
int mmcblk_evaluateResponse(MmcblkResponse_t *response);

#endif
