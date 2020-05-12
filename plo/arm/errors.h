/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Error codes
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
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

#ifndef _ERRORS_H_
#define _ERRORS_H_


#define ERR_NONE             0
#define ERR_ARG             -1
#define ERR_UNSUPPORTED     -2

#define ERR_LOW_BIOS        -16
#define ERR_LOW_MMSIZE      -17

#define ERR_STD_NOTEQ       -32

#define ERR_TIMEOUT  -48

#define ERR_MSG_IO          -64

#define ERR_PHFS_IO         -80
#define ERR_PHFS_PROTO      -81
#define ERR_PHFS_FILE       -82

#define ERR_BUSY            -90
#define ERR_FLASH_IO        -91

#define ERR_MMC_IO          -71
#endif
