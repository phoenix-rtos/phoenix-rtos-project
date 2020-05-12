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
#ifndef _PLO_EFI_H_
#define _PLO_EFI_H_

#include "cpu.h"


/* EFI status/error codes */
#define EFI_ERROR(x)			(0x80000000|(x))
#define EFI_SUCCESS			0
#define EFI_LOAD_ERROR			EFI_ERROR(1)
#define EFI_INVALID_PARAMETER		EFI_ERROR(2)
#define EFI_BAD_BUFFER_SIZE		EFI_ERROR(4)
#define EFI_BUFFER_TOO_SMALL		EFI_ERROR(5)
#define EFI_OUT_OF_RESOURCES		EFI_ERROR(9)
#define EFI_INCOMPATIBLE_VERSION	EFI_ERROR(25)

#ifndef __ASSEMBLY__

/* (U)EFI == Microsoft */
#define EFIAPI __attribute__((ms_abi))

/* argument markers used in EFI spec */
#define IN /* input */
#define OUT /* output */

/* types and structures from EFI spec; somewhat pruned */

typedef u16 wchar_t;
typedef void *EFI_HANDLE;
typedef u32 EFI_STATUS;
typedef unsigned UINTN;	/* native uint */
typedef u64 EFI_PHYSICAL_ADDRESS;
typedef u64 EFI_VIRTUAL_ADDRESS;

/* EFI boolean: requires values: 0 or 1 (others: undefined) */
typedef u8 BOOLEAN;
#define TRUE (BOOLEAN)1
#define FALSE (BOOLEAN)0


typedef struct {
	u32 Data1;
	u16 Data2;
	u16 Data3;
	u8 Data4[8];
} EFI_GUID;


typedef struct {
	u64 Signature;
	u32 Revision;
	u32 HeaderSize;
	u32 crc32;
	u32 reserved;
} EFI_TABLE_HEADER;


typedef struct {
	u32			 Type;
	u32			 pad;
	EFI_PHYSICAL_ADDRESS	 PhysicalStart;
	EFI_VIRTUAL_ADDRESS	 VirtualStart;
	u64			 NumberOfPages;
	u64			 Attribute;
} EFI_MEMORY_DESCRIPTOR;


/* struct typedefs */

typedef void EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef void EFI_RUNTIME_SERVICES;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef void SIMPLE_TEXT_OUTPUT_MODE;
typedef struct _EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef void EFI_CONFIGURATION_TABLE;
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;


/* console output */

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING) (
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN wchar_t const			*String
);

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	void				*Reset;
	EFI_TEXT_STRING			 OutputString;
	void				*TestString;
	void				*QueryMode;
	void				*SetMode;
	void				*SetAttribute;
	void				*ClearScreen;
	void				*SetCursorPosition;
	void				*EnableCursor;
	SIMPLE_TEXT_OUTPUT_MODE		*Mode;
};


/* EFI filesystem path */

struct _EFI_DEVICE_PATH_PROTOCOL {
	u8 Type;
	u8 SubType;
	u8 Length[2];
};

#define EFI_PATH_END	0x7F


/* EFI boot services */

#define EFI_BOOT_SERVICES_SIGNATURE	((u64)0x56524553544f4f42ull)

typedef enum {
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

#define EFI_MEMORY_WB	0x0000000000000008

#define EFI_MEMORY_DESCRIPTOR_VERSION 1


/* EFI image info */

typedef struct {
	u32				 Revision;
	EFI_HANDLE			 ParentHandle;
	EFI_SYSTEM_TABLE		*SystemTable;

	EFI_HANDLE			 DeviceHandle;
	void				*FilePath;
	void				*Reserved;

	u32				 LoadOptionsSize;
	wchar_t				*LoadOptions;

	void				*ImageBase;
	u64				 ImageSize;
	EFI_MEMORY_TYPE			 ImageCodeType;
	EFI_MEMORY_TYPE			 ImageDataType;
	void				*Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
	{ 0x5B1B31A1, 0x9562, 0x11d2, {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B} }


/* EFI file system access */

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN) (
	IN EFI_FILE_PROTOCOL	*This,
	OUT EFI_FILE_PROTOCOL	**NewHandle,
	IN wchar_t const	*FileName,
	IN u64			 OpenMode,
	IN u64			 Attributes
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE) (
	IN EFI_FILE_PROTOCOL	*This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ) (
	IN EFI_FILE_PROTOCOL	*This,
	IN OUT UINTN		*BufferSize,
	OUT void		*Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION) (
	IN EFI_FILE_PROTOCOL	*This,
	OUT u64			*Position
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION) (
	IN EFI_FILE_PROTOCOL	*This,
	IN u64			 Position
);

struct _EFI_FILE_PROTOCOL {
	u64			 Revision;
	EFI_FILE_OPEN		 Open;
	EFI_FILE_CLOSE		 Close;
	void			*Delete;
	EFI_FILE_READ		 Read;
	void			*Write;
	EFI_FILE_GET_POSITION	 GetPosition;
	EFI_FILE_SET_POSITION	 SetPosition;
	void			*GetInfo;
	void			*SetInfo;
	void			*Flush;
};


typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME) (
	IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL	*This,
	OUT EFI_FILE_PROTOCOL			**Root
);

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
	u64						 Revision;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME	 OpenVolume;
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION	0x00010000
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	{ 0x0964e5b22, 0x6459, 0x11d2, {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b} }

#define EFI_FILE_MODE_READ	0x0000000000000001


/* EFI boot services */

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES) (
	IN EFI_ALLOCATE_TYPE		 Type,
	IN EFI_MEMORY_TYPE		 MemoryType,
	IN UINTN			 Pages,
	IN OUT EFI_PHYSICAL_ADDRESS	*Memory
);

typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES) (
	IN EFI_PHYSICAL_ADDRESS		 Memory,
	IN UINTN			 Pages
);

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP) (
	IN OUT UINTN			*MemoryMapSize,
	IN OUT EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	OUT UINTN			*MapKey,
	OUT UINTN			*DescriptorSize,
	OUT u32				*DescriptorVersion
);

typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL) (
	IN EFI_HANDLE			 Handle,
	IN const EFI_GUID		*Protocol,
	OUT void *			*Interface
);

typedef EFI_STATUS (EFIAPI *EFI_EXIT) (
	IN EFI_HANDLE			 ImageHandle,
	IN EFI_STATUS			 ExitStatus,
	IN UINTN			 ExitDataSize,
	IN wchar_t			*ExitData
);

typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES) (
	IN EFI_HANDLE			 ImageHandle,
	IN UINTN			 MapKey
);


struct _EFI_BOOT_SERVICES {
	EFI_TABLE_HEADER Hdr;

	void			*RaiseTPL;
	void			*RestoreTPL;

	EFI_ALLOCATE_PAGES	 AllocatePages;
	EFI_FREE_PAGES		 FreePages;
	EFI_GET_MEMORY_MAP	 GetMemoryMap;
	void			*AllocatePool;
	void			*FreePool;

	void			*CreateEvent;
	void			*SetTimer;
	void			*WaitForEvent;
	void			*SignalEvent;
	void			*CloseEvent;
	void			*CheckEvent;

	void			*InstallProtocolInterface;
	void			*ReinstallProtocolInterface;
	void			*UninstallProtocolInterface;
	EFI_HANDLE_PROTOCOL	 HandleProtocol;
	void			*reserved;
	void			*RegisterProtocolNotify;
	void			*LocateHandle;
	void			*LocateDevicePath;
	void			*InstallConfigurationTable;

	void			*LoadImage;
	void			*StartImage;
	EFI_EXIT		 Exit;
	void			*UnloadImage;
	EFI_EXIT_BOOT_SERVICES	 ExitBootServices;

	void			*GetNextMonotonicCount;
	void			*Stall;
	void			*SetWatchdogTimer;

	void			*ConnectController;
	void			*DisconnectController;

	void			*OpenProtocol;
	void			*CloseProtocol;
	void			*OpenProtocolInformation;

	void			*ProtocolsPerHandle;
	void			*LocateHandleBuffer;
	void			*LocateProtocol;
	void			*InstallMultipleProtocolInterfaces;
	void			*UninstallMultipleProtocolInterfaces;

	void			*CalculateCrc32;

	void			*CopyMem;
	void			*SetMem;
	void			*CreateEventEx;
};


/* EFI system table */

struct _EFI_SYSTEM_TABLE {
	EFI_TABLE_HEADER		 Hdr;
	wchar_t				*FirmwareVendor;
	u32				 FirmwareRevision;
	EFI_HANDLE			 ConsoleInHandle;
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL	*ConIn;
	EFI_HANDLE			 ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*ConOut;
	EFI_HANDLE			 StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*StdErr;
	EFI_RUNTIME_SERVICES		*RuntimeServices;
	EFI_BOOT_SERVICES		*BootServices;
	UINTN				 NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE		*ConfigurationTable;
};

#define EFI_SYSTEM_TABLE_SIGNATURE	((u64)0x5453595320494249ull)

#endif /* __ASSEMBLY__ */

#endif /* _PLO_EFI_H_ */
