/*
 * Phoenix-RTOS
 *
 * Task manager implementation
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/threads.h>
#include <sys/time.h>

#include <wasm_export.h>

#include "mqtt.h"
#include "taskman.h"
#include "config.h"
#include "base64.h"
#include "log.h"


struct {
	struct proplet_task tasks[PROPLET_MAX_WASM_TASKS];
	int monitored[PROPLET_MAX_MONITORED_TASKS];
} taskman_context;


void *task_thread(void *private)
{
	struct proplet_task *task = (struct proplet_task *)private;
	struct proplet_start_request *request;
	wasm_module_t module;
	wasm_exec_env_t exec;
	wasm_func_type_t function;

	task->tid = gettid();

	for (;;) {
		pthread_mutex_lock(&task->lock);
		pthread_cond_wait(&task->ready, &task->lock);

		request = task->request;
		gettime(&task->starttime, NULL);

		wasm_runtime_init_thread_env();

		log_debug("Thread %d woken up", task->tid);

		module = wasm_runtime_load(request->code, request->codesz, task->errorbuf, sizeof(task->errorbuf));
		if (module == NULL) {
			plt_mqtt_publish_results(request->id, -1, task->errorbuf);
			log_error("Failed to load WASM runtime for task %s: %s", request->id, task->errorbuf);
			plt_obj_dealloc_start_request(task->request);
			task->request = NULL;
			pthread_mutex_unlock(&task->lock);
			atomic_store(&task->executing, false);
			wasm_runtime_destroy_thread_env();
			continue;
		}

		log_debug("Thread %d loaded WASM runtime", task->tid);

		task->inst = wasm_runtime_instantiate(module, PROPLET_WASM_STACK_SIZE, PROPLET_WASM_HEAP_SIZE, task->errorbuf, sizeof(task->errorbuf));
		if (task->inst == NULL) {
			plt_mqtt_publish_results(request->id, -1, task->errorbuf);
			log_error("Failed to instantiate WASM module for task %s: %s", request->id, task->errorbuf);
			wasm_runtime_unload(module);
			plt_obj_dealloc_start_request(task->request);
			task->request = NULL;
			pthread_mutex_unlock(&task->lock);
			atomic_store(&task->executing, false);
			wasm_runtime_destroy_thread_env();
			continue;
		}

		log_debug("Thread %d loaded WASM runtime", task->tid);

		function = wasm_runtime_lookup_function(task->inst, "main");
		if (function == NULL) {
			plt_mqtt_publish_results(request->id, -1, "function \"main\" not found");
			log_error("Could not find \"main\" symbol in WASM module of task %s", request->id);
			wasm_runtime_deinstantiate(task->inst);
			wasm_runtime_unload(module);
			plt_obj_dealloc_start_request(task->request);
			task->request = NULL;
			pthread_mutex_unlock(&task->lock);
			atomic_store(&task->executing, false);
			wasm_runtime_destroy_thread_env();
			continue;
		}

		log_debug("Thread %d found WASM main function", task->tid);

		exec = wasm_runtime_create_exec_env(task->inst, PROPLET_WASM_STACK_SIZE);
		if (exec == NULL) {
			plt_mqtt_publish_results(request->id, -1, "could not create WASM execution environment");
			log_error("Could not create WASM execution environment for task %s", request->id);
			wasm_runtime_deinstantiate(task->inst);
			wasm_runtime_unload(module);
			plt_obj_dealloc_start_request(task->request);
			task->request = NULL;
			pthread_mutex_unlock(&task->lock);
			atomic_store(&task->executing, false);
			wasm_runtime_destroy_thread_env();
			continue;
		}

		log_debug("Thread %d created WASM execution environment", task->tid);

		/* We need to unlock here, so that we can get interrupted */
		pthread_mutex_unlock(&task->lock);

		/*
		 * @FIXME(michal.lach): this call will fault if the WASM function
		 * expects more arguments than provided.
		 *
		 * Maybe fixable with cmake flag configuration for WAMR.
		 */
		if (wasm_runtime_call_wasm(exec, function, request->argc, request->argv)) {
			plt_mqtt_publish_results(request->id, request->argv[0], NULL);
			log_info("Task %s finished with result %d", request->id, request->argv[0]);
		}
		else {
			log_info("Task %s terminated with an exception: %s", request->id, wasm_runtime_get_exception(task->inst));
			if (strcmp((char *)wasm_runtime_get_exception(task->inst), "Exception: terminated by user") != 0) {
				plt_mqtt_publish_results(request->id, request->argv[0], (char *)wasm_runtime_get_exception(task->inst));
			}
		}

		pthread_mutex_lock(&task->lock);

		wasm_runtime_destroy_exec_env(exec);
		wasm_runtime_deinstantiate(task->inst);
		wasm_runtime_unload(module);
		plt_obj_dealloc_start_request(task->request);
		task->request = NULL;
		pthread_mutex_unlock(&task->lock);
		atomic_store(&task->executing, false);
		wasm_runtime_destroy_thread_env();
	}
}


static unsigned long djb2(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}


void plt_taskman_init(void)
{
	RuntimeInitArgs args = { 0 };
	int i;

	/*
	 * Dependant on the target node, threads might be expensive,
	 * we probably want to restrict the maximum amount user can
	 * spawn.
	 */
	args.max_thread_num = PROPLET_WASM_MAX_THREADS;

	/*
	 * WAMR uses the system allocator to allocate heaps for applets.
	 * Applets initialized with wasm_runtime_instantiate() with heap_size passed,
	 * will allocate heap_size bytes of memory for usage inside of the applet.
	 */
	args.mem_alloc_type = Alloc_With_System_Allocator;
	wasm_runtime_full_init(&args);

	memset(&taskman_context, 0, sizeof(taskman_context));
	for (i = 0; i < sizeof(taskman_context.tasks) / sizeof(*taskman_context.tasks); i++) {
		taskman_context.tasks[i].index = i;
		atomic_init(&taskman_context.tasks[i].executing, false);
		pthread_cond_init(&taskman_context.tasks[i].ready, NULL);
		pthread_mutex_init(&taskman_context.tasks[i].lock, NULL);
		pthread_create(&taskman_context.tasks[i].thread, NULL, task_thread, &taskman_context.tasks[i]);
	}

	for (i = 0; i < sizeof(taskman_context.monitored) / sizeof(*taskman_context.monitored); i++) {
		taskman_context.monitored[i] = PROPLET_TASKMAN_EMPTY_MONITORING_SLOT;
	}
}


static inline struct proplet_task *plt_taskman_task_find(char *id)
{
	unsigned long hash = djb2((unsigned char *)id) % PROPLET_MAX_WASM_TASKS, i = hash;
	struct proplet_task *thread = &taskman_context.tasks[hash], *next = NULL;

	if (memcmp(thread->request->id, id, PROPLET_PROPELLER_MAX_ID_SIZE) != 0) {
		do {
			i++;
			i %= PROPLET_MAX_WASM_TASKS;
			next = &taskman_context.tasks[i];
		} while (memcmp(next->request->id, id, PROPLET_PROPELLER_MAX_ID_SIZE) != 0 || next != thread);

		if (next == thread) {
			return NULL;
		}

		thread = next;
	}

	return thread;
}


static inline struct proplet_task *plt_taskman_suggest_free_task_slot(char *suggestion)
{
	unsigned long hash = djb2((unsigned char *)suggestion) % PROPLET_MAX_WASM_TASKS, i = hash;
	struct proplet_task *task = &taskman_context.tasks[hash], *next = NULL;

	if (atomic_load(&task->executing)) {
		do {
			i++;
			i %= PROPLET_MAX_WASM_TASKS;
			next = &taskman_context.tasks[i];
		} while ((atomic_load(&task->executing)) || (next != task));

		if (next == task) {
			return NULL;
		}

		task = next;
	}

	return task;
}


int plt_taskman_register_task(struct proplet_start_request *request)
{
	int i;
	struct proplet_task *task = plt_taskman_suggest_free_task_slot(request->id);

	if (task == NULL) {
		return -ENOSPC;
	}

	task->request = request;
	for (i = 0; i < sizeof(taskman_context.monitored) / sizeof(*taskman_context.monitored); i++) {
		if (taskman_context.monitored[i] == PROPLET_TASKMAN_EMPTY_MONITORING_SLOT) {
			break;
		}
	}

	if (i < sizeof(taskman_context.monitored) / sizeof(*taskman_context.monitored)) {
		taskman_context.monitored[i] = task->index;
	}

	atomic_store(&task->executing, true);
	pthread_cond_signal(&task->ready);

	return 0;
}


int plt_taskman_stop_task(char *id)
{
	struct proplet_task *task = plt_taskman_task_find(id);

	if (task == NULL) {
		return -EINVAL;
	}

	if (!atomic_load(&task->executing)) {
		return -EBUSY;
	}

	pthread_mutex_lock(&task->lock);
	wasm_runtime_terminate(task->inst);
	pthread_mutex_unlock(&task->lock);

	return 0;
}


void plt_taskman_sample_metrics(void)
{
	int idx, i;
	double sum;
	time_t uptime;
	threadinfo_t tinfo;
	struct proplet_task *task;

	gettime(&uptime, NULL);

	for (i = 0; i < sizeof(taskman_context.monitored) / sizeof(*taskman_context.monitored); i++) {
		idx = taskman_context.monitored[i];
		if (idx == PROPLET_TASKMAN_EMPTY_MONITORING_SLOT) {
			continue;
		}

		task = &taskman_context.tasks[idx];
		if (!atomic_load(&task->executing)) {
			taskman_context.monitored[i] = PROPLET_TASKMAN_EMPTY_MONITORING_SLOT;
			continue;
		}

		threadinfo(task->tid, PH_THREADINFO_CPUTIME, &tinfo);

		task->metrics.snapshot.cpu_percent = ((double)(tinfo.cpuTime) / (double)uptime) * 100.0f;
		task->metrics.snapshot.uptime_seconds = (uptime - task->starttime) / 1000000;

		/*
		 * These would require having a separate implementation of standard
		 * library features such as a custom allocator, thread library or IO
		 * library that would gather these metrics
		 *
		 * Additionally, a matter of disk access from the WASM app was not
		 * seriously considered.
		 */
		task->metrics.snapshot.memory_bytes = 0;
		task->metrics.snapshot.memory_percent = 0.0f;
		task->metrics.snapshot.disk_read_bytes = 0;
		task->metrics.snapshot.disk_written_bytes = 0;
		task->metrics.snapshot.thread_count = 1;
		task->metrics.snapshot.fd_count = 0;

		if (task->metrics.snapshot.cpu_percent > task->metrics.aggregated.max_cpu_usage) {
			task->metrics.aggregated.max_cpu_usage = task->metrics.snapshot.cpu_percent;
		}

		sum = task->metrics.aggregated.avg_cpu_usage * (double)task->metrics.aggregated.sample_count;
		sum += task->metrics.snapshot.cpu_percent;
		task->metrics.aggregated.avg_cpu_usage = sum / (double)(task->metrics.aggregated.sample_count + 1);

		task->metrics.aggregated.sample_count++;
	}
}


void plt_taskman_publish_task_metrics(void)
{
	int idx, i;
	struct proplet_task *task;

	for (i = 0; i < sizeof(taskman_context.monitored) / sizeof(*taskman_context.monitored); i++) {
		idx = taskman_context.monitored[i];
		if (idx == PROPLET_TASKMAN_EMPTY_MONITORING_SLOT) {
			continue;
		}

		task = &taskman_context.tasks[idx];
		if (!atomic_load(&task->executing)) {
			taskman_context.monitored[i] = PROPLET_TASKMAN_EMPTY_MONITORING_SLOT;
			continue;
		}

		plt_mqtt_publish_task_metrics(task->request->id, &task->metrics);
	}
}
