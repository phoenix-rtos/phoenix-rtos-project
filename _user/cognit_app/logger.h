/*
 * Phoenix-RTOS
 *
 * Logging for COGNIT demo
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#ifndef COGNIT_APP_LOGGER_H
#define COGNIT_APP_LOGGER_H

#include <stdio.h>

#define COGNIT_APP_LOG_LVL_DEBUG (0)
#define COGNIT_APP_LOG_LVL_INFO  (1)
#define COGNIT_APP_LOG_LVL_WARN  (2)
#define COGNIT_APP_LOG_LVL_ERROR (3)

#ifndef COGNIT_APP_LOG_LVL
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_WARN
#endif

#define COL_GREY   "\033[1;30m"
#define COL_RED    "\033[1;31m"
#define COL_GREEN  "\033[1;32m"
#define COL_YELLOW "\033[1;33m"
#define COL_BLUE   "\033[1;34m"
#define COL_CYAN   "\033[1;36m"
#define COL_VIOLET "\033[1;35m"
#define COL_NORMAL "\033[0m"

#define log_debug(fmt, ...) \
	do { \
		if (COGNIT_APP_LOG_LVL <= COGNIT_APP_LOG_LVL_DEBUG) { \
			fprintf(stderr, COL_GREY COGNIT_APP_LOG_TAG fmt COL_NORMAL "\n", ##__VA_ARGS__); \
		} \
	} while (0)

#define log_info(fmt, ...) \
	do { \
		if (COGNIT_APP_LOG_LVL <= COGNIT_APP_LOG_LVL_INFO) { \
			fprintf(stderr, COL_NORMAL COGNIT_APP_LOG_TAG fmt COL_NORMAL "\n", ##__VA_ARGS__); \
		} \
	} while (0)

#define log_success(fmt, ...) \
	do { \
		if (COGNIT_APP_LOG_LVL <= COGNIT_APP_LOG_LVL_WARN) { \
			fprintf(stderr, COL_GREEN COGNIT_APP_LOG_TAG fmt COL_NORMAL "\n", ##__VA_ARGS__); \
		} \
	} while (0)

#define log_warn(fmt, ...) \
	do { \
		if (COGNIT_APP_LOG_LVL <= COGNIT_APP_LOG_LVL_WARN) { \
			fprintf(stderr, COL_YELLOW COGNIT_APP_LOG_TAG fmt COL_NORMAL "\n", ##__VA_ARGS__); \
		} \
	} while (0)

#define log_error(fmt, ...) \
	do { \
		if (COGNIT_APP_LOG_LVL <= COGNIT_APP_LOG_LVL_ERROR) { \
			fprintf(stderr, COL_RED COGNIT_APP_LOG_TAG fmt COL_NORMAL "\n", ##__VA_ARGS__); \
		} \
	} while (0)

#endif /* COGNIT_APP_LOGGER_H */
