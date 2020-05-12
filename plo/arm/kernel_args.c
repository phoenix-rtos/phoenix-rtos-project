/* 
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Kernel arguments
 *
 * Copyright 2001, 2005 Pawel Pisarczyk
 * Copyright 2006 Radoslaw F. Wawrzusiak
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

#include "kernel_args.h"
#include "low.h"
#include "plostd.h"

void add_kernel_arg(const char *key, const char *value)
{
	low_memcpy(plo_syspage.args + plo_syspage.argsize, key, plostd_strlen(key));
	plo_syspage.argsize += plostd_strlen(key);
	*(plo_syspage.args + plo_syspage.argsize) = '=';
	plo_syspage.argsize++;
	low_memcpy(plo_syspage.args + plo_syspage.argsize, value, plostd_strlen(value));
	plo_syspage.argsize += plostd_strlen(value);
	*(plo_syspage.args + plo_syspage.argsize) = 0;
	plo_syspage.argsize++;
	
	plo_syspage.argc++;
}

void add_default_kernel_args(char *default_args)
{
	int i;
	for (i = 0; i < plostd_strlen(default_args); ++i) {
		if (default_args[i] != ' ')
			*(plo_syspage.args + plo_syspage.argsize) = default_args[i];
		else {
			*(plo_syspage.args + plo_syspage.argsize) = 0;
			plo_syspage.argc++;
		}

		plo_syspage.argsize++;
	}
	*(plo_syspage.args + plo_syspage.argsize) = 0;
	plo_syspage.argsize++;
	plo_syspage.argc++;
}
