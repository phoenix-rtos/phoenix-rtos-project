#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "../types.h"

extern int verifySignature(u8 *sign_type, u8 *sign_data, u64 data_start_blk, u64 data_size, int pdn, s32 h);

#endif
