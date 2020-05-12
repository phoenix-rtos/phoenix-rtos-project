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

#ifndef _FLASH_H_
#define _FLASH_H_

#include "types.h"

extern s32 flash_open(u16 fn, char *name, u32 flags);
extern s32 flash_read(u16 fn, s32 handle, u64 *pos, u8 *buff, u32 len);
extern s32 flash_close(u16 fn, s32 handle);
extern void flash_init(void);
extern void flash_done(void);

#endif
