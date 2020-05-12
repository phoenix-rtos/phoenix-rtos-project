#include <dev/mmcblk/io/mmcblk_spi.h>
#include <dev/mmcblk/mmcblk_priv.h>

void mmcblk_spi_init(void *cardPtr)
{
	assert(!"Not implemented");
}

int mmcblk_spi_sendCommand(void *cardPtr, u32 cmd, u32 cmd_arg, s32 block_num, u16 block_size, void *admaDT)
{
	assert(!"Not implemented");
	return 0;
}

MmcblkResponse_t mmcblk_spi_waitForResponse(void *cardPtr, int cmd)
{
	MmcblkResponse_t ret;
	memset(&ret, 0x0, sizeof(ret));
	assert(!"Not implemented");
	return ret;
}

int mmcblk_spi_transferWait(void *cardPtr)
{
	assert(!"Not implemented");
	return 0;
}

int mmcblk_spi_waitBusy(void *cardPtr)
{
	assert(!"Not implemented");
	return 0;
}

void mmcblk_spi_freeDMA(FreePtr *fp)
{
	assert(!"Not implemented");
}

void *mmcblk_spi_setupDMA(void *cardPtr, void *bufptr, s32 bufsize, FreePtr **fp, char *htBuf)
{
	assert(!"Not implemented");
	return NULL;
}

int mmcblk_spi_switchHighSpeed(void *cardPtr, u32 baudrate)
{
	assert(!"Not implemented");
	return 0;
}

int mmcblk_spi_setupBaudRate(void *cardPtr, u32 baudrate)
{
	assert(!"Not implemented");
	return 0;
}

int mmcblk_spi_setupBusWidth(void *cardPtr, MmcblkBusWidth_t width)
{
	assert(!"Not implemented");
	return 0;
}

int mmcblk_spi_setupEndian(void *cardPtr, MmcblkEndian_t endian)
{
	assert(!"Not implemented");
	return 0;
}
