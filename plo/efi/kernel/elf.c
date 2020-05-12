/*
 * Phoenix-RTOS
 *
 * Operating system kernel
 *
 * Processes management
 *
 * Copyright 2012-2015 Phoenix Systems
 * Copyright 2001, 2006-2007 Pawel Pisarczyk
 * Author: Pawel Pisarczyk, Pawel Kolodziej, Pawel Krezolek
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include HAL
#include "errno.h"
#include "elf.h"

#define ELFCLASS_VALUE 1	/* ELFCLASS32 */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ELFDATA_VALUE 1		/* ELFDATA2LSB */
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ELFDATA_VALUE 2		/* ELFDATA2MSB */
#else
#error "No byte order defined"
#endif

#define ELFVER_VALUE 1		/* EV_CURRENT */
#define EHDRVER_VALUE 1		/* EV_CURRENT (EV_SYSV) */

/* NOTE: Don't use globals here, as this might be called from boot code
 *	 running w/o paging enabled or with 1:1 mapping */

/* Checks ELF header format; return <0 on error */
int proc_check_elf_hdr(Elf32_Ehdr *ehdr, size_t size)
{
	size_t psz;

	/* Test ELF header */

	if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
		return -EINVAL;
	if (ehdr->e_ident[4] != ELFCLASS_VALUE || ehdr->e_ident[5] != ELFDATA_VALUE || ehdr->e_ident[6] != ELFVER_VALUE)
		return -EINVAL;
	if (ehdr->e_machine != HAL_ELF_MACHINE || ehdr->e_version != ELFVER_VALUE)
		return -EINVAL;

	/* Test ELF program header */

	if (!ehdr->e_phoff || ehdr->e_phoff >= size)
		return -EINVAL;
	if (ehdr->e_phentsize < sizeof(Elf32_Phdr) || ehdr->e_phentsize > 0x100 /* arbitrary */)
		return -EINVAL;
	if (ehdr->e_phnum > 255 /* arbitrary */)
		return -EINVAL;

	psz = ehdr->e_phnum * ehdr->e_phentsize;

	if (psz >= size || ehdr->e_phoff + psz > size)
		return -EINVAL;

	return 0;
}


/* Returns number of PT_LOAD segments, or <0 on error */
int proc_check_elf_phdr(Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, size_t size)
{
	int i, n;

	/* Test ELF segments */

	n = 0;
	for (i = 0; i < ehdr->e_phnum; ++i, phdr = (void *)phdr + ehdr->e_phentsize) {
		if (phdr->p_type != PT_LOAD)
			continue;
		if (phdr->p_filesz) {
			if (phdr->p_filesz > size || phdr->p_offset >= size)
				return -EINVAL;
			if (phdr->p_offset + phdr->p_filesz > size)
				return -EINVAL;
		}
		++n;
	}

	return n;
}
