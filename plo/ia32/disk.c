/*
 * Phoenix-RTOS
 *
 * plo - operating system loader
 *
 * Disk routines
 *
 * Copyright 2012 Phoenix Systems
 * Copyright 2001, 2005, 2006 Pawel Pisarczyk
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "config.h"
#include "errors.h"
#include "low.h"
#include "plostd.h"
#include "timer.h"
#include "serial.h"
#include "disk.h"


s32 disk_open(u16 dn, char *name, u32 flags)
{
	return 64;
}


/* Function reads bytes from file specified by starting block number (handle) */
s32 disk_read(u16 dn, s32 handle, u32 *pos, u8 *buff, u32 len)
{
	char *cache = CACHE_OFFS;
	static u8 empty = 1, lh, loffs;
	static u16 ldn = -1, lc;
	char *sector;
	u8 h, s, heads, secs, offs;
	u16 c, cyls;
	u32 sb, eb;
	u16 boffs, size, i;
	int err;

	if (!len)
		return ERR_ARG;

	/* Get disk geometry */
	if (err = low_int13param(dn, &cyls, &heads, &secs))
		return ERR_PHFS_IO;

	cyls += 1;
	heads += 1;

	/* Calculate start and end block numbers */
	sb = handle + (*pos) / SECTOR_SIZE;
	eb = handle + ((*pos) + len - 1) / SECTOR_SIZE;
	boffs = (u16)(*pos % SECTOR_SIZE);

	for (;;) {
		c = sb / secs / heads;
		h = (sb / secs) % heads;
		s = 1 + (sb % secs);

		offs = 1 + ((s - 1) / CACHE_SIZE) * CACHE_SIZE;

		/* Read track from disk from cache */
		if (empty || (dn != ldn) || (c != lc) || (h != lh) || (offs != loffs)) {
			if ((err = low_int13read(dn, c, h, offs, min(CACHE_SIZE, secs), cache))) {
				return ERR_PHFS_IO;
			}

			/* plostd_printf(ATTR_DEBUG, "disk_read: empty=%d ldn=%d lc=%d lh=%d loffs=%d dn=%d c=%d h=%d offs=%d\n", empty, ldn, lc, lh, loffs, dn, c, h, offs); */

			empty = 0;
			ldn = dn;
			lc = c;
			lh = h;
			loffs = offs;
		}
		sector = &cache[((s - 1) % CACHE_SIZE) * SECTOR_SIZE];

		if (sb == eb)
			size = ((*pos + len - 1) % SECTOR_SIZE) - boffs + 1;
		else
			size = SECTOR_SIZE - boffs;

		low_memcpy(buff, (char *)(sector + boffs), size);

		buff += size;
		boffs = 0;

		if (sb++ == eb)
			break;
	}

	*pos += size;
	return 0;
}


s32 disk_close(u16 dn, s32 handle)
{
	return 0;
}
