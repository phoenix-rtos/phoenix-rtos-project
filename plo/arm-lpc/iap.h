/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * IAP Commands
 *
 * Copyright 2006 Radoslaw F. Wawrzusiak
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

#ifndef _IAP_H_
#define _IAP_H_

#include "types.h"

/* IAP Status Codes */
#define IAP_CMD_SUCCESS		0x00	/* Command is executed successfully.	*/
#define IAP_INVALID_CMD		0x01	/* Invalid command.						*/
#define	IAP_SRC_ADDR_ERROR	0x02
#define	IAP_DST_ADDR_ERROR	0x03
#define	IAP_SRC_ADDR_NMAP	0x04
#define	IAP_DST_ADDR_NMAP	0x05
#define	IAP_COUNT_ERROR		0x06
#define	IAP_INVALID_SECTOR	0x07
#define	IAP_SECTOR_NBLANK	0x08
#define	IAP_SECTOR_NPREP	0x09
#define	IAP_COMPARE_ERROR	0x0A
#define	IAP_BUSY			0x0B

#define	IAP_ERR				0xF4

extern int iap_partId();
extern int iap_erase(void*, u32);
extern int iap_copyto(void*, void*, u32);

#endif
