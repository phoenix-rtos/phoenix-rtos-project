/*
 * Phoenix-RTOS
 *
 * base64 encoder and decoder - header
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifer: BSD-3-Clause
 */

#ifndef _PH_PROPLET_BASE64_H
#define _PH_PROPLET_BASE64_H

#include <stddef.h>
#include <stdint.h>


extern uint8_t b64table[];


extern uint8_t b64rev[];


int base64_encode(const uint8_t *in, size_t insz, uint8_t *out, size_t outsz);


int base64_decode(const uint8_t *in, size_t insz, uint8_t *out, size_t outsz);


#endif /* _PH_PROPLET_BASE64_H */
