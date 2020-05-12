/*
 * plo - phoenix system loader, EFI version
 *
 * Copyright 2017 Phoenix Systems
 * Author: Michał Mirosław
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include "efi.h"
#include "elf.h"
#include "multiboot.h"
#include "string.h"

#include <stdarg.h>


#define ENABLE_DUMP_MEMMAP


#define PRINT_CONV_BUFLEN 32
#define MAX_CMDLINE 1024
#define MAX_FILENAME 512

#define PLO_VERSION "1"

static const wchar_t default_cfgname[] = L"phoenix.cfg";
static const char default_kernel[] = "phoenix.elf";


/* parameters passed from EFI */
typedef struct {
	EFI_SYSTEM_TABLE *sys;
	EFI_HANDLE img_handle;
	EFI_LOADED_IMAGE_PROTOCOL *info;
	EFI_FILE_PROTOCOL *esp_fs;
} efi_params_t;


static unsigned efi_strnlen(const wchar_t *s, unsigned n)
{
	unsigned i = 0;

	while (n-- && *s++)
		++i;

	return i;
}


static void efi_outputString(efi_params_t *efi, const wchar_t *s)
{
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *con = efi->sys->ConOut;

	con->OutputString(con, s);
}


static void efi_print(efi_params_t *efi, const char *s)
{
	wchar_t buf[PRINT_CONV_BUFLEN + 1], *p = buf, *end = buf + sizeof(buf)/sizeof(*buf) - 1;

	while (*s) {
		*p++ = (u8)*s++;
		if (p != end)
			continue;

		*p = 0;
		efi_outputString(efi, buf);
		p = buf;
	}

	if (p != buf) {
		*p = 0;
		efi_outputString(efi, buf);
	}
}


static void efi_printHex(efi_params_t *efi, u32 value)
{
	wchar_t buf[2+8+1];
	wchar_t *p = buf + sizeof(buf)/sizeof(buf[0]);

	*--p = 0;

	do {
		u16 c = value & 0xF;
		value >>= 4;

		c += c >= 10 ? 'A' - 10 : '0';
		*--p = c;
	} while (value);

	*--p = 'x';
	*--p = '0';

	efi_outputString(efi, p);
}


__attribute__((sentinel))
static void efi_println(efi_params_t *efi, ...)
{
	va_list ap;
	char *s;

	va_start(ap, efi);

	while ((s = va_arg(ap, char *)))
		efi_print(efi, s);

	va_end(ap);

	efi_outputString(efi, L"\r\n");
}


static void efi_printHexln(efi_params_t *efi, const char *msg, u32 value)
{
	efi_print(efi, msg);
	efi_printHex(efi, value);
	efi_outputString(efi, L"\r\n");
}


static void efi_printWstrln(efi_params_t *efi, const char *msg, const wchar_t *str)
{
	efi_print(efi, msg);
	efi_outputString(efi, str);
	efi_outputString(efi, L"\r\n");
}


__attribute__((noreturn)) __attribute__((cold))
static void efi_bail(efi_params_t *efi, EFI_STATUS err, const char *msg)
{
	efi_println(efi, "[PLO] ", msg, " failed", NULL);
	efi->sys->BootServices->Exit(efi->img_handle, err, 0, NULL);

	efi_print(efi, "Buggy firmware -- returned from Exit()\r\n");
	for(;;)
		efi->sys->BootServices->Exit(efi->img_handle, err, 0, NULL);
}


static void efi_warn(efi_params_t *efi, const char *msg)
{
	efi_println(efi, "WARNING: ", msg, NULL);
}


static void efi_assertFail(efi_params_t *efi, const char *what, const char *how, u32 tested, u32 expected)
{
	efi_println(efi, "When checking ", what, how, ":", NULL);
	efi_printHexln(efi, "       got: ", tested);
	efi_printHexln(efi, "  expected: ", expected);
	efi_bail(efi, EFI_INVALID_PARAMETER, "Assertion");
}


static void efi_assertEq(efi_params_t *efi, const char *what, u32 tested, u32 expected)
{
	if (tested != expected)
		efi_assertFail(efi, what, " for equality", tested, expected);
}


static void efi_assertMin(efi_params_t *efi, const char *what, u32 tested, u32 expected)
{
	if (tested < expected)
		efi_assertFail(efi, what, " lower bound", tested, expected);
}


static void *efi_allocPagesCall(efi_params_t *efi, unsigned n, u32 req_addr, EFI_ALLOCATE_TYPE mode)
{
	EFI_PHYSICAL_ADDRESS addr = req_addr;
	EFI_STATUS err;
	u32 va;

	if ((err = efi->sys->BootServices->AllocatePages(mode, EfiLoaderData, n, &addr)) != EFI_SUCCESS)
		efi_bail(efi, err, "AllocatePages()");

	va = (u32)addr;
	return (void *)va;
}


static void *efi_allocPages(efi_params_t *efi, unsigned n)
{
	return efi_allocPagesCall(efi, n, 0, AllocateAnyPages);
}


static void *efi_allocPagesAt(efi_params_t *efi, unsigned n, u32 req_addr)
{
	void *addr = efi_allocPagesCall(efi, n, req_addr, AllocateAddress);
	efi_assertEq(efi, "pinned address", (u32)addr, req_addr);
	return addr;
}


static void *efi_alloc(efi_params_t *efi, unsigned sz)
{
	efi_printHexln(efi, "Alloc pages for ", sz);
	return efi_allocPages(efi, (sz + 0xFFF) >> 12);
}


static void efi_freePages(efi_params_t *efi, void *addr, unsigned n)
{
	u32 va = (u32)addr;

	efi->sys->BootServices->FreePages(va, n);
}


static void efi_free(efi_params_t *efi, void *addr, unsigned sz)
{
	efi_freePages(efi, addr, (sz + 0xFFF) >> 12);
}


static size_t efi_getMemoryMapSize(efi_params_t *efi, size_t *out_desc_sz)
{
	EFI_BOOT_SERVICES *efi_bs = efi->sys->BootServices;
	EFI_STATUS err;
	UINTN sz, desc_sz, mmap_key;
	u32 ver;

	sz = 0;
	if ((err = efi_bs->GetMemoryMap(&sz, NULL, &mmap_key, &desc_sz, &ver)) != EFI_BUFFER_TOO_SMALL)
		efi_bail(efi, err, "GetMemoryMap() for size");

	efi_assertEq(efi, "mmap version", ver, EFI_MEMORY_DESCRIPTOR_VERSION);
	efi_assertMin(efi, "mmap entry size", desc_sz, sizeof(EFI_MEMORY_DESCRIPTOR));

	/* include space for new entry containing map itself */
	*out_desc_sz = desc_sz;
	return sz + desc_sz;
}


static size_t efi_getMemoryMap(efi_params_t *efi, EFI_MEMORY_DESCRIPTOR *buf, size_t bufsz, UINTN *mmap_key)
{
	EFI_BOOT_SERVICES *efi_bs = efi->sys->BootServices;
	EFI_STATUS err;
	UINTN sz, desc_sz;
	u32 ver;

	sz = bufsz;
	if ((err = efi_bs->GetMemoryMap(&sz, buf, mmap_key, &desc_sz, &ver)) != EFI_SUCCESS)
		efi_bail(efi, err, "GetMemoryMap()");

	return sz / desc_sz;
}


static void efi_convertMemoryMap(efi_params_t *efi, multiboot_info_t *mb, UINTN *mmap_key)
{
	EFI_MEMORY_DESCRIPTOR *map;
	multiboot_memory_map_t *last;
	size_t desc_sz, sz, n;

	sz = efi_getMemoryMapSize(efi, &desc_sz);
	sz = (sz + 3) & ~3;

	map = efi_alloc(efi, sz + (sz / desc_sz) * sizeof(*last));
	last = (void *)map + sz;

	n = efi_getMemoryMap(efi, map, sz, mmap_key);

	efi_printHexln(efi, "Memory block count from EFI: ", n);

	mb->mmap_length = n * sizeof(*last);
	mb->mmap_addr = (u32)last;

	for (; n--; ++last, map = (void *)map + desc_sz) {
		last->size = sizeof(*last) - sizeof(last->size);
		last->addr = map->PhysicalStart;
		last->len = (u64)map->NumberOfPages << 12;

		switch (map->Type) {
		case EfiLoaderCode:
		case EfiLoaderData:
		case EfiBootServicesCode:
		case EfiBootServicesData:
		case EfiConventionalMemory:
			if (map->Attribute & EFI_MEMORY_WB)
				last->type = MULTIBOOT_MEMORY_AVAILABLE;
			else
				last->type = MULTIBOOT_MEMORY_RESERVED;
			break;
		case EfiACPIReclaimMemory:
			last->type = MULTIBOOT_MEMORY_ACPI_RECLAIMABLE;
			break;
		case EfiACPIMemoryNVS:
			last->type = MULTIBOOT_MEMORY_NVS;
			break;
                case EfiUnusableMemory:
			last->type = MULTIBOOT_MEMORY_BADRAM;
			break;
		default:
			last->type = MULTIBOOT_MEMORY_RESERVED;
			break;
		}
#ifdef ENABLE_DUMP_MEMMAP
		efi_print(efi, "Block at ");
		efi_printHex(efi, last->addr);
		efi_print(efi, " len ");
		efi_printHex(efi, last->len);
		efi_print(efi, " type ");
		efi_printHex(efi, last->type);
		efi_outputString(efi, L"\r\n");
#endif
	}
}


/* retrieves config file name (first param) and converts the rest of
 * the command line to ASCII [assumes no chars > 127 are present] */
static void efi_processCommandLine(efi_params_t *efi, const wchar_t **cfgname, const char **cmdline)
{
	unsigned cmdsz;
	wchar_t *p;
	char *q;

	*cfgname = default_cfgname;
	*cmdline = "";

	if (!efi->info->LoadOptionsSize)
		return;

	p = efi->info->LoadOptions;
	cmdsz = efi_strnlen(p, efi->info->LoadOptionsSize / 2);
	if (cmdsz == efi->info->LoadOptionsSize / 2) {
		efi_warn(efi, "got unterminated command line from EFI");
		/* may overwrite past the and - hope for the best */
		p[cmdsz] = 0;
	}

	efi_printWstrln(efi, "Command line from EFI: ", efi->info->LoadOptions);

	while (cmdsz && *p != ' ')
		--cmdsz, ++p;

	while (cmdsz && *p == ' ')
		--cmdsz, ++p;
	if (!cmdsz)
		return;

	/* lone '-' -> default */
	if (p[0] != '-' || (cmdsz > 1 && p[1] != ' '))
		*cfgname = p;

	while (cmdsz && *p != ' ')
		--cmdsz, ++p;
	*p++ = 0;

	while (cmdsz && *p == ' ')
		--cmdsz, ++p;
	if (!cmdsz)
		return;

	*cmdline = q = (char *)p;
	while (cmdsz--)
		*q++ = (u8)*p++;
	*q++ = 0;
}


static void efi_releaseBoot(efi_params_t *efi, UINTN mmap_key)
{
	EFI_STATUS err;

	if ((err = efi->sys->BootServices->ExitBootServices(efi->img_handle, mmap_key)) != EFI_SUCCESS)
		efi_bail(efi, err, "ExitBootServices()");
}


static void efi_getImageInfo(efi_params_t *efi)
{
	static const EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_STATUS err;
	void *ptr;

	if ((err = efi->sys->BootServices->HandleProtocol(efi->img_handle, &guid, &ptr)) != EFI_SUCCESS)
		efi_bail(efi, err, "HandleProtocol() for Loaded-Image");

	efi->info = ptr;
}


static void efi_getEspFilesystem(efi_params_t *efi)
{
	static const EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
	EFI_STATUS err;
	void *ptr;

	if ((err = efi->sys->BootServices->HandleProtocol(efi->info->DeviceHandle, &guid, &ptr)) != EFI_SUCCESS)
		efi_bail(efi, err, "HandleProtocol() for Simple-FS");

	fs = ptr;

	if ((err = fs->OpenVolume(fs, &efi->esp_fs)) != EFI_SUCCESS)
		efi_bail(efi, err, "OpenVolume() for ESP");

	if (!efi->esp_fs)
		efi_bail(efi, EFI_INVALID_PARAMETER, "ESP NULL check");
}


static void efi_init(efi_params_t *efi)
{
	efi_printHexln(efi, "\r\n[PLO] ver. " PLO_VERSION " started by EFI version ", efi->sys->Hdr.Revision);

	efi_getImageInfo(efi);
	efi_printHexln(efi, "Loaded at ", (u32)efi->info->ImageBase);

	if (efi->sys->Hdr.Signature != EFI_SYSTEM_TABLE_SIGNATURE)
		efi_warn(efi, "bad System Table signature");
	if (efi->sys->BootServices->Hdr.Signature != EFI_BOOT_SERVICES_SIGNATURE)
		efi_warn(efi, "bad Boot Services signature");

	efi_getEspFilesystem(efi);
}


static void *efi_readFile(efi_params_t *efi, const wchar_t *fname, unsigned *file_sz)
{
	EFI_FILE_PROTOCOL *fh;
	EFI_STATUS err;
	u64 sz;
	UINTN rsz;
	void *ptr;

	efi_printWstrln(efi, "Reading ", fname);

	if ((err = efi->esp_fs->Open(efi->esp_fs, &fh, fname, EFI_FILE_MODE_READ, 0)) != EFI_SUCCESS)
		efi_bail(efi, err, "Open()");

	if ((err = fh->SetPosition(fh, ~(u64)0)) != EFI_SUCCESS)
		efi_bail(efi, err, "Seek() to end");

	if ((err = fh->GetPosition(fh, &sz)) != EFI_SUCCESS)
		efi_bail(efi, err, "GetSize()");

	if ((err = fh->SetPosition(fh, 0)) != EFI_SUCCESS)
		efi_bail(efi, err, "Seek() to 0");

	efi_printHexln(efi, "File size = ", sz);
	if (file_sz)
		*file_sz = sz;

	/* alloc 1 byte more for terminating NUL */
	ptr = efi_alloc(efi, rsz = sz + 1);

	if ((err = fh->Read(fh, &rsz, ptr)) != EFI_SUCCESS)
		efi_bail(efi, err, "Read()");

	fh->Close(fh);

	((char *)ptr)[sz] = 0;

	return ptr;
}


static void *efi_readFileA(efi_params_t *efi, const char *fname, unsigned *file_sz)
{
	wchar_t fn[MAX_FILENAME], *p;
	size_t n = MAX_FILENAME-1;

	p = fn;
	while (*fname && n--)
		*p++ = (u8)*fname++;
	*p = 0;

	return efi_readFile(efi, fn, file_sz);
}


static multiboot_info_t *plo_allocBootData(efi_params_t *efi, size_t n_modules)
{
	multiboot_info_t *mb;

	mb = efi_alloc(efi, sizeof(*mb) + n_modules * sizeof(multiboot_module_t) + MAX_CMDLINE);
	mb->flags = MULTIBOOT_INFO_CMDLINE | MULTIBOOT_INFO_MODS | MULTIBOOT_INFO_MEM_MAP;
	mb->mods_count = n_modules;
	mb->mods_addr = (u32)mb + sizeof(*mb);
	mb->cmdline = mb->mods_addr + n_modules * sizeof(multiboot_module_t);

	return mb;
}


static size_t plo_countLines(const char *data)
{
	const char *last = data;
	size_t n = 0;

	for (; *data; ++data) {
		if (*data != '\n')
			continue;

		++n;
		last = data + 1;
	}

	return n + (last != data);
}


static inline int plo_isspace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


static char *plo_skipPastEol(char *p)
{
	while (*p && *p != '\n')
		++p;
	if (*p)
		++p;
	return p;
}

static int plo_parseConfigLine(char **config, char **fname, size_t *fname_len)
{
	static const char *const config_keywords[] = { "kernel", "module" };
	char *p, *q, *keyword;
	int i;

	for (p = *config; *p; ++p)
		if (!plo_isspace(*p))
			break;
	if (!*p)
		return 0;

	keyword = p;
	for (; *p; ++p)
		if (plo_isspace(*p))
			break;

	if (!*p)
		return -1;

	for (i = 0; i < 2; ++i)
		if (!hal_strncmp(keyword, config_keywords[i], p - keyword))
			break;

	if (i == 2) {
		*config = plo_skipPastEol(p);
		return -1;
	}

	for (; *p && *p != '\n'; ++p)
		if (!plo_isspace(*p))
			break;

	if (!*p || *p == '\n')
		return -1;
	*fname = p;

	for (; *p; ++p)
		if (plo_isspace(*p))
			break;

	*fname_len = p - *fname;

	*config = q = plo_skipPastEol(p);
	for (; q != p; *q-- = 0)
		if (!plo_isspace(*q))
			break;

	return i + 1;
}


static void plo_appendCmdline(efi_params_t *efi, multiboot_info_t *mb, const char *cmdline)
{
	size_t n, i;
	char *p;

	p = (void *)mb->cmdline;
	i = hal_strlen(p);
	n = MAX_CMDLINE-1 - i;

	if (i && n)
		p[n--, i++] = ' ';

	hal_strncpy(p + i, cmdline, n);

	if (hal_strlen(cmdline) > n) {
		efi_warn(efi, "kernel command line truncated");
		p[MAX_CMDLINE-1] = 0;
	}
}


static void plo_parseConfig(efi_params_t *efi, multiboot_info_t *mb, char *config, const char **kname)
{
	multiboot_module_t *mod;
	char *fn, *kernel, *kernel_args;
	size_t n_cmds, fn_len;
	int i;

	mod = (void *)mb->mods_addr;
	kernel_args = "";
	kernel = NULL;
	n_cmds = 0;

	for (;;) {
		i = plo_parseConfigLine(&config, &fn, &fn_len);
		if (!i)
			break;

		++n_cmds;
		if (i < 0) {
			efi_printHexln(efi, "Bad command #", n_cmds);
			continue;
		}

		switch (i) {
		case 1: /* kernel */
			if (kernel) {
				efi_printHexln(efi, "Duplicate kernel at cmd #", n_cmds);
				continue;
			}

			kernel = fn;
			if (kernel[fn_len]) {
				kernel_args = kernel + fn_len;
				while (*kernel_args != '\n' && plo_isspace(*kernel_args))
					++kernel_args;
				kernel[fn_len] = 0;
			}
			break;

		case 2: /* module */
			mod->cmdline = (u32)fn;
			mod->pad = fn_len;
			++mod;
			break;

		default:
			__builtin_unreachable();
		}
	}

	*kname = kernel ? kernel : default_kernel;
	mb->mods_count = mod - (multiboot_module_t *)mb->mods_addr;
	plo_appendCmdline(efi, mb, kernel_args);
}


static unsigned plo_checkElf(efi_params_t *efi, void *elf_data, size_t elf_sz)
{
	Elf32_Ehdr *ehdr = elf_data;
	int n;

	if (proc_check_elf_hdr(ehdr, elf_sz))
		efi_bail(efi, EFI_LOAD_ERROR, "ELF header check");

	if ((n = proc_check_elf_phdr(ehdr, elf_data + ehdr->e_phoff, elf_sz)) < 0)
		efi_bail(efi, EFI_LOAD_ERROR, "ELF program header check");

	return (unsigned)n;
}


static void *plo_loadKernel(efi_params_t *efi, multiboot_info_t *mb, const char *fn)
{
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	void *data, *segment, *entry;
	size_t fsz, sz, n;

	data = efi_readFileA(efi, fn, &fsz);
	n = plo_checkElf(efi, data, fsz);
	ehdr = data;
	phdr = data + ehdr->e_phoff;

	for (; n; ++phdr) {
		if (phdr->p_type != PT_LOAD)
			continue;
		--n;

		sz = (phdr->p_memsz + 0xFFF) >> 12;
		segment = efi_allocPagesAt(efi, sz, phdr->p_paddr);
		sz <<= 12;
		hal_memcpy(segment, data + phdr->p_offset, phdr->p_filesz);
		hal_memset(segment + phdr->p_filesz, 0, sz - phdr->p_filesz);

		efi_print(efi, "Kernel at ");
		efi_printHex(efi, (u32)segment);
		efi_print(efi, " len ");
		efi_printHex(efi, sz);
		efi_print(efi, " load ");
		efi_printHex(efi, phdr->p_filesz);
		efi_outputString(efi, L"\r\n");
	}

	entry = (void *)ehdr->e_entry;
	efi_printHexln(efi, "Kernel entry at ", (u32)entry);

	efi_free(efi, data, fsz + 1);

	return entry;
}


static void plo_loadModule(efi_params_t *efi, multiboot_module_t *mod)
{
	size_t sz;
	char c, *fn;
	void *data;

	fn = (void *)mod->cmdline;
	c = fn[mod->pad];
	fn[mod->pad] = 0;

	data = efi_readFileA(efi, fn, &sz);
	mod->mod_start = (u32)data;
	mod->mod_end = mod->mod_start + sz;

	fn[mod->pad] = c;
	mod->pad = 0;

	plo_checkElf(efi, data, sz);
}


static void *plo_load(efi_params_t *efi, multiboot_info_t *mb, const char *kernel)
{
	multiboot_module_t *mod;
	void *entry;
	size_t n;

	entry = plo_loadKernel(efi, mb, kernel);

	mod = (void *)mb->mods_addr;
	for (n = 0; n < mb->mods_count; ++n)
		plo_loadModule(efi, mod + n);

	return entry;
}


static void plo_buildBootData(efi_params_t *efi, multiboot_info_t **out_mb, const char **kernel)
{
	multiboot_info_t *mb;
	const wchar_t *cfgname;
	const char *cmdline;
	char *config;

	efi_processCommandLine(efi, &cfgname, &cmdline);

	config = efi_readFile(efi, cfgname, NULL);

	*out_mb = mb = plo_allocBootData(efi, plo_countLines(config));

	*(char *)mb->cmdline = 0;
	plo_appendCmdline(efi, mb, cmdline);

	plo_parseConfig(efi, mb, config, kernel);
}


static void efi_run(efi_params_t *efi)
{
	multiboot_info_t *mb;
	const char *kernel;
	void *entry;
	UINTN mmap_key;

	plo_buildBootData(efi, &mb, &kernel);
	entry = plo_load(efi, mb, kernel);
	efi_convertMemoryMap(efi, mb, &mmap_key);

	efi_println(efi, "[PLO] Starting kernel...", NULL);
	efi_println(efi, "Cmdline: ", (char *)mb->cmdline, NULL);
	efi_releaseBoot(efi, mmap_key);
	asm volatile(
		"jmp *%0"
	:
	: "r" (entry), "a" (MULTIBOOT_BOOTLOADER_MAGIC), "b" (mb)
	: "memory");
}


EFIAPI EFI_STATUS efi_main(EFI_HANDLE my_handle, EFI_SYSTEM_TABLE *sys)
{
	efi_params_t efi = { sys, my_handle, };

	efi_init(&efi);
	efi_run(&efi);

	return EFI_LOAD_ERROR;
}
