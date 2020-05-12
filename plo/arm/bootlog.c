#include "types.h"
#include "flash.h"
#include "low.h"
#include "plostd.h"

#define CURR_VER 1
#define BOOT_DEV_LEN 22
#define ERASE_SIZE 4096
#define LOG_OFFSET (120 * 1024)
#define LOG_SIZE (2 * ERASE_SIZE / sizeof(bootlog_entry_t))

typedef struct {
	u16 ver;
	u16 seq;
	u32 date;
	char boot_dev[BOOT_DEV_LEN];
	u16 crc;
} bootlog_entry_t;


typedef struct {
	bootlog_entry_t data[LOG_SIZE];
	unsigned index[LOG_SIZE];
	unsigned used;
} bootlog_t;


static void _bootlog_sort(bootlog_entry_t log[], unsigned log_index[], int count)
{
	int seq, d, pivot, p = 0, i;

	if (count < 2)
		return;

	pivot = log[log_index[count - 1]].seq;
	for (i = 0; i < count - 1; i++) {
		seq = log[log_index[i]].seq;
		if (seq < pivot)
			d = seq + 0xffff - pivot;
		else
			d = seq - pivot;

		if (d < 0x8000) {
			d = log_index[i];
			log_index[i] = log_index[p];
			log_index[p] = d;
			p++;
		}
	}
	d = log_index[count - 1];
	log_index[count - 1] = log_index[p];
	log_index[p] = d;

	_bootlog_sort(log, log_index, p);
	_bootlog_sort(log, log_index + p + 1, count - p - 1);
}


static u16 _bootlog_crc16(const u8* data, unsigned len)
{
	u16 i, crc = 0xffff;

	while (len--) {
		i = (crc >> 8 ^ *data++) & 0xff;
		i ^= i >> 4;
		crc = (crc << 8) ^ (i << 12) ^ (i << 5) ^ i;
	}

	return crc;
}


int bootlog_read(bootlog_t *plog)
{
	u64 offs = LOG_OFFSET;
	int res, i;
	s32 fd;

	plog->used = 0;

	fd = flash_open(0, NULL, 0);
	if (fd == -1)
		return -1;

	res = flash_read(0, fd, &offs, (u8 *)plog->data, LOG_SIZE * sizeof(bootlog_entry_t));
	if (res == LOG_SIZE * sizeof(bootlog_entry_t))
		for (i = 0; i < LOG_SIZE; i++) {
			if (plog->data[i].ver <= CURR_VER && plog->data[i].seq > 0 && plog->data[i].seq < 0xffff &&
					_bootlog_crc16((u8 *)(plog->data + i), sizeof(bootlog_entry_t) - 2) == plog->data[i].crc)
				plog->index[plog->used++] = i;
		}

	flash_close(0, fd);
	_bootlog_sort(plog->data, plog->index, plog->used);

	return plog->used;
}


bootlog_entry_t *bootlog_get(bootlog_t *plog, unsigned num)
{
	if (num >= plog->used)
		return NULL;

	return plog->data + plog->index[num];
}


int bootlog_search(bootlog_t *plog, u16 seq)
{
	int d, mid = 0, first = 0, last = plog->used;
	u16 mid_seq;

	while (first != last) {
		mid = (last + first) / 2;
		mid_seq = plog->data[plog->index[mid]].seq;
		if (mid_seq < seq)
			d = mid_seq + 0xffff - seq;
		else
			d = mid_seq - seq;

		if (d == 0)            /* seq == mid_seq */
			break;
		else if (d < 0x8000) { /* seq < mid_seq */
			if (first == mid)
				break;
			else
				first = mid;
		}
		else {                 /* seq > mid_seq */
			if (last == mid)
				break;
			else
				last = mid;
		}
	}
	return mid;
}


const char *bootlog_dev(void)
{
	static 	bootlog_t plog;
	static const bootlog_entry_t dflt_ent = { .ver = CURR_VER, .seq = 1, .boot_dev = "mmcblk0p1" };
	u32 persist;
	const bootlog_entry_t *ent;
	int i, p;

	bootlog_read(&plog);
	persist = low_persistGet();

	if (persist == 0) {
		/* Power up, select boot device with highest sequence number. */
		ent = bootlog_get(&plog, 0);
		if (ent == NULL)
			ent = &dflt_ent;
		low_persistSet(ent->seq);
	}
	else if (persist < (2 << 16)) {
		ent = bootlog_get(&plog, bootlog_search(&plog, persist & 0xffff));
		if (ent == NULL)
			ent = &dflt_ent;
		low_persistSet(persist + (1 << 16));
	}
	else {
		p = bootlog_search(&plog, persist & 0xffff);
		do {
			p++;
			ent = bootlog_get(&plog, p);
			if (ent == NULL) {
				/* If list of boot devices is exhausted, try start from the beginning. */
				if ((ent = bootlog_get(&plog, 0)) == NULL)
					ent = &dflt_ent;
				break;
			}
			for (i = 0; i < p; i++)
				if (plostd_strcmp(ent->boot_dev, bootlog_get(&plog, i)->boot_dev) == 0)
					break;
		}
		while(i != p);
		low_persistSet(ent->seq);
	}

	return ent->boot_dev;
}
