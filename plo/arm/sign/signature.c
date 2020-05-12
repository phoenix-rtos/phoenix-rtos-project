#include "signature.h"
#include "sha1.h"
#define NULL 0

typedef struct {
	const char *name;
	int (*verify)(u8 *sign, u32 dataStart, u32 dataSize, int pdn, s32 h);
} Signature_t;


const static Signature_t signaturing[] = {
	{
		"crc16",
		NULL
	},
	{
		"crc32",
		NULL
	},
	{
		"sha1",
		sha1verify
	},
	{
		"sha224",
		NULL
	},
	{
		"sha256",
		NULL
	},
	{
		"sha384",
		NULL
	},
	{
		"sha512",
		NULL
	},
	{
		"md5",
		NULL
	}
};


int verifySignature(u8 *sign_type, u8 *sign_data, u64 data_start_blk, u64 data_size, int pdn, s32 h)
{
	int sign, ch;
	for(sign = 0; sign < (sizeof(signaturing) / sizeof(signaturing[0])); ++sign) {
		if(signaturing[sign].verify != NULL) {
			for(ch = 0; signaturing[sign].name[ch]; ++ch) {
				if(signaturing[sign].name[ch] != sign_type[ch])
					break;
			}
			if(!signaturing[sign].name[ch]) {
				return signaturing[sign].verify(sign_data, data_start_blk, data_size, pdn, h);
			}
		}
	}
	return -1;
}
