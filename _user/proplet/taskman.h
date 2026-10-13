/*
 * Phoenix-RTOS
 *
 * Task manager - header
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PH_PROPLET_TASKMAN_H
#define _PH_PROPLET_TASKMAN_H

#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>

#include <wasm_export.h>

#include "config.h"
#include "dto.h"


#define PROPLET_TASKMAN_EMPTY_MONITORING_SLOT -1


struct proplet_task_metrics {
	struct {
		double cpu_percent;
		size_t memory_bytes;
		double memory_percent;
		size_t disk_read_bytes;
		size_t disk_written_bytes;
		time_t uptime_seconds;
		size_t thread_count;
		size_t fd_count;
	} snapshot;
	struct {
		double avg_cpu_usage;
		double max_cpu_usage;
		double avg_memory_usage;
		double max_memory_usage;
		size_t total_disk_read;
		size_t total_disk_written;
		size_t sample_count;
	} aggregated;
};


struct proplet_task {
	/* These fields should be immutable for the entire clients life */
	int tid;
	int index;
	pthread_t thread;
	/* Conditional variable used for sleeping until the request is prepared by the main thread */
	pthread_cond_t ready;
	pthread_mutex_t lock;
	time_t starttime;

	/*
	 * This is atomic and not synchronized with mutex, so that the MQTT thread can freely check
	 * if a task slot is taken or not.
	 */
	/* clang-format off */
	_Atomic bool executing;
	/* clang-format on */

	/*
	 * request is prepared by the MQTT thread and shall be valid and immutable (to every thread)
	 * for the entire task execution time, and shall be disposed of by the task thread once task
	 * execution finishes.
	 */
	struct proplet_start_request *request;

	/*
	 * Task metrics are synchronized with mutex in this record. They should be reset once task
	 * execution finishes.
	 */
	struct proplet_task_metrics metrics;

	/*
	 * This field is synchronized with mutex in this record, it shall remain valid until the tasks
	 * execution time and be desposed of by the task thread once task execution finishes.
	 */
	wasm_module_inst_t inst;

	/*
	 * This field is unsynchronized, it shall only be used by the task thread.
	 */
	char errorbuf[PROPLET_WASM_ERROR_BUFFER_SIZE];
};


/*
 * Initializes the WAMR runtime and proplet WASM executor workers
 */
void plt_taskman_init(void);

/*
 * Caches the task start request internally, the task can be later invoked
 * with later plt_wasm_kickoff_task() after acquiring the WASM code from proxy or
 * immediately, if the code was provided inline.
 *
 * After invocation, request parameter is handed over to thread specific handler,
 * which will free it after finishing the task. The caller shan't modify any part
 * of value passed by pointer in request after a successful call to plt_wasm_register_task()
 */
int plt_taskman_register_task(struct proplet_start_request *request);


/*  */
int plt_taskman_stop_task(char *id);


void plt_taskman_sample_metrics(void);


void plt_taskman_publish_task_metrics(void);


#endif /* _PH_PROPLET_TASKMAN_H */
