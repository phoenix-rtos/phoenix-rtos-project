/*
 * Phoenix-RTOS
 *
 * Logging
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PH_PROPLET_LOG_H
#define _PH_PROPLET_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <syslog.h>


extern bool PLT_LOGGING_VERBOSE;


#define fail(fmt, ...) \
	do { \
		fprintf(stderr, "error: " fmt "\n", ##__VA_ARGS__); \
		exit(EXIT_FAILURE); \
	} while (0);


#ifdef PH_PROPLET_SYSLOG_LOGGING


#define log_info(fmt, ...) \
	syslog(LOG_INFO, "[INFO ]: " fmt "\n", ##__VA_ARGS__); \
	conslog("[INFO ]: " fmt "\n", ##__VA_ARGS__);


#define log_error(fmt, ...) \
	syslog(LOG_ERR, "[ERROR]: " fmt "\n", ##__VA_ARGS__); \
	conslog("[ERROR]: " fmt "\n", ##__VA_ARGS__);


#define log_debug(fmt, ...) \
	if (PLT_LOGGING_VERBOSE) { \
		syslog(LOG_DEBUG, "[DEBUG]" fmt "\n", ##__VA_ARGS__); \
		conslog("[DEBUG]: " fmt "\n", ##__VA_ARGS__); \
	}


#else

#define log_info(fmt, ...) conslog("[INFO  ]: " fmt "\n", ##__VA_ARGS__);


#define log_error(fmt, ...) conslog("[ERROR ]: " fmt "\n", ##__VA_ARGS__);


#define log_debug(fmt, ...) conslog("[DEBUG]: " fmt "\n", ##__VA_ARGS__);


#endif


static inline void conslog(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}


#endif /* _PH_PROPLET_LOG_H */
