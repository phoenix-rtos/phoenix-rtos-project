/*
 * Phoenix-RTOS
 *
 * armplo - operating system loader for ARM7TDMI
 *
 * IAP Commands
 *
 * Copyright 2006 Radoslaw F. Wawrzusiak
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

#include "low.h"
#include "iap.h"

#define IAP_LOCATION		0x7ffffff1

/* IAP Command codes */
#define	IAP_PREPARE			0x32
#define	IAP_RAM_FLASH		0x33
#define	IAP_ERASE			0x34
#define	IAP_BLANK_CHECK		0x35
#define	IAP_PART_ID			0x36
#define	IAP_VERSION			0x37
#define	IAP_COMPARE			0x38
#define	IAP_ISP				0x39

typedef struct {
	u32	code;
	union {
		u32	  start;
		void* dst;
	};

	union {
		u32	  end;
		void* src;
	};

	u32	count;
	u32	cclk;

} iap_input_t;

typedef struct {
	u32 status;
	union {
		u32	offset;
		u32	partId;
		u32 version;
	};

	u32 content;
} iap_return_t;

typedef void (*IAP)(iap_input_t* ,iap_return_t*);


IAP iap_entry = (IAP) IAP_LOCATION;


int iap_partId()
{
	iap_input_t in;
	iap_return_t out;

	in.code = IAP_PART_ID;
	low_cli();
	iap_entry(&in, &out);
	low_sti();

	if(out.status == IAP_CMD_SUCCESS){
		return out.partId;
	}
	return -out.status;
}


iap_erase(void* dst, u32 len){
	iap_input_t in;
	iap_return_t out;

	if((u32) dst < 0x8000){
		in.start = (u32) dst >> 12;
		if(((u32) dst & 0xfff) + len > 0x1000){
			in.end = in.start + 1;
		}else{
			in.end = in.start;
		}
	}else if((u32) dst < 0x78000){
		in.start = ((u32) dst >> 15) + 7;
		if(((u32) dst & 0x7fff) + len > 0x8000){
			in.end = in.start + 1;
		}else{
			in.end = in.start;
		}
	}else if((u32) dst < 0x7d000){
		in.start = ((u32) dst >> 12) - 0x62;
		if(((u32) dst & 0xfff) + len > 0x1000){
			in.end = in.start + 1;
			if(in.end > 26) return IAP_ERR;
		}else{
			in.end = in.start;
		}
	}else{
		return IAP_ERR;
	}

	in.code = IAP_PREPARE;
	low_cli();
	do{
		iap_entry(&in, &out);
	}while(out.status == IAP_BUSY);
	low_sti();

	if(out.status != IAP_CMD_SUCCESS){
		return -out.status;
	}

	in.code = IAP_ERASE;
	in.count = 60000;
	low_cli();
	do{
		iap_entry(&in, &out);
	}while(out.status == IAP_BUSY);
	low_sti();

	return -out.status;
}


iap_copyto(void* dst, void* src, u32 len)
{
	iap_input_t in;
	iap_return_t out;

	in.code = IAP_PREPARE;
	if((u32) dst < 0x8000){
		in.start = (u32) dst >> 12;
		if(((u32) dst & 0xfff) + len > 0x1000){
			in.end = in.start + 1;
		}else{
			in.end = in.start;
		}
	}else if((u32) dst < 0x78000){
		in.start = ((u32) dst >> 15) + 7;
		if(((u32) dst & 0x7fff) + len > 0x8000){
			in.end = in.start + 1;
		}else{
			in.end = in.start;
		}
	}else if((u32) dst < 0x7d000){
		in.start = ((u32) dst >> 12) - 0x62;
		if(((u32) dst & 0xfff) + len > 0x1000){
			in.end = in.start + 1;
			if(in.end > 26) return IAP_ERR;
		}else{
			in.end = in.start;
		}
	}else{
		return IAP_ERR;
	}

	low_cli();
	do{
		iap_entry(&in, &out);
	}while(out.status == IAP_BUSY);
	low_sti();

	if(out.status != IAP_CMD_SUCCESS){
		return -out.status;
	}


	in.code = IAP_RAM_FLASH;
	in.dst = dst;
	in.src = src;
	in.count = len;
	in.cclk = 60000;

	low_cli();
	do{
		iap_entry(&in, &out);
	}while(out.status == IAP_BUSY);
	low_sti();

	return -out.status;
}
