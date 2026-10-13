/*
 * Phoenix-RTOS
 *
 * MQTT data transfer objects, serialization and deserialization
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <malloc.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/threads.h>

#include <jansson.h>

#include "config.h"
#include "dto.h"
#include "log.h"
#include "base64.h"
#include "taskman.h"


struct proplet_start_request *plt_json_deserialize_start(char *buf, size_t sz, char **imgref)
{
	int i;
	bool latent = false;
	uint32_t *argv = NULL, argc = 0, codesz = 0;
	const char *output = NULL, *name, *id;
	json_t *root, *cur, *it;
	struct proplet_start_request *request;

	if ((buf == NULL) || (sz == 0) || (imgref == NULL)) {
		return NULL;
	}

	root = json_loadb(buf, sz, 0, NULL);
	if ((root == NULL) || (!json_is_object(root))) {
		return NULL;
	}

	cur = json_object_get(root, "id");
	if ((cur == NULL) || (!json_is_string(cur)) || (strlen(json_string_value(cur)) > PROPLET_PROPELLER_MAX_ID_SIZE)) {
		json_decref(root);
		return NULL;
	}

	id = json_string_value(cur);

	cur = json_object_get(root, "name");
	if ((cur == NULL) || (!json_is_string(cur))) {
		json_decref(root);
		return NULL;
	}

	name = json_string_value(cur);

	cur = json_object_get(root, "inputs");
	if ((cur != NULL) && (json_is_array(cur))) {
		argv = calloc(json_array_size(cur) * sizeof(*argv), 1);
		if (argv == NULL) {
			json_decref(root);
			return NULL;
		}

		argc = json_array_size(cur);
		for (i = 0; i < argc; i++) {
			it = json_array_get(cur, i);
			if (!json_is_string(it)) {
				continue;
			}
			argv[i] = atoi(json_string_value(it));
		}
	}

	cur = json_object_get(root, "latent");
	if ((cur != NULL) && (json_is_boolean(cur))) {
		latent = json_boolean_value(cur);
	}

	cur = json_object_get(root, "image_id");
	if ((cur != NULL) && (json_is_string(cur))) {
		*imgref = strdup(json_string_value(cur));
	}
	else {
		cur = json_object_get(root, "file");
		if ((cur == NULL) || (!json_is_string(cur))) {
			json_decref(root);
			free(argv);
			return NULL;
		}

		output = json_string_value(cur);
		if (strlen(output) > PROPLET_MAX_INLINE_PAYLOAD_SIZE) {
			json_decref(root);
			free(argv);
			return NULL;
		}

		codesz = base64_decode((const uint8_t *)output, strlen(output), NULL, 0);
	}

	request = calloc(sizeof(*request), 1);
	if (request == NULL) {
		json_decref(root);
		free(argv);
		return NULL;
	}

	memcpy(request->id, id, PROPLET_PROPELLER_MAX_ID_SIZE);
	request->argv = argv;
	request->argc = argc;
	request->latent = latent;
	request->name = strdup(name);
	request->codesz = codesz;
	if (request->codesz > 0) {
		request->code = malloc(codesz);
		if (request->code == NULL) {
			json_decref(root);
			free(argv);
			free(request->code);
			free(request);
			return NULL;
		}

		base64_decode((const uint8_t *)output, strlen(output), request->code, codesz);
	}
	else {
		request->code = NULL;
	}

	json_decref(root);

	return request;
}


struct proplet_stop_request *plt_json_deserialize_stop(char *buf, size_t sz)
{
	const char *id;
	json_t *root, *cur;
	struct proplet_stop_request *request;

	if ((buf == NULL) || (sz == 0)) {
		return NULL;
	}

	root = json_loadb(buf, sz, 0, NULL);
	if (root == NULL || !json_is_object(root)) {
		return NULL;
	}

	cur = json_object_get(root, "id");
	if (cur == NULL || !json_is_string(cur) || strlen(json_string_value(cur)) > PROPLET_PROPELLER_MAX_ID_SIZE) {
		json_decref(root);
		return NULL;
	}
	else {
		id = json_string_value(cur);
	}

	request = calloc(sizeof(*request), 1);
	if (request == NULL) {
	}

	memcpy(request->id, id, strlen(id));
	json_decref(root);

	return request;
}


char *plt_json_serialize_discovery(char *proplet_id, char *description, char *tags,
		char *location, char *address)
{
	json_t *root = json_object(), *metadata = json_object();
	char *output, hostname[255];
	meminfo_t minfo;

	gethostname(hostname, sizeof(hostname));

	meminfo(&minfo);

	json_object_set_new(root, "proplet_id", json_string(proplet_id));
	json_object_set_new(root, "namespace", json_string(PROPLET_NAMESPACE));

	json_object_set_new(metadata, "description", json_string(description));
	json_object_set_new(metadata, "tags", json_string(tags));
	json_object_set_new(metadata, "location", json_string(location));
	json_object_set_new(metadata, "ip", json_string(address));
	json_object_set_new(metadata, "environment", json_string(PROPLET_ENVIRONMENT));
	json_object_set_new(metadata, "os", json_string(PROPLET_HOST_OS));
	json_object_set_new(metadata, "hostname", json_string(hostname));
	json_object_set_new(metadata, "cpu_arch", json_string(PROPLET_HOST_ARCH));
	json_object_set_new(metadata, "total_memory_bytes", json_integer(minfo.page.alloc + minfo.page.free));
	json_object_set_new(metadata, "proplet_version", json_string(PROPLET_VERSION));
	json_object_set_new(metadata, "wasm_runtime", json_string(PROPLET_WASM_RUNTIME));

	json_object_set_new(root, "metadata", metadata);

	output = json_dumps(root, JSON_COMPACT);
	json_decref(root);

	return output;
}


char *plt_json_serialize_system_metrics(char *proplet_id, int timestamp)
{
	int ret, tid;
	char *output;
	json_t *root = json_object(), *cmetrics = json_object(), *mmetrics = json_object();
	time_t uptime;
	double cpu = 0.0f, utime, stime;
	meminfo_t minfo;
	threadinfo_t tinfo;
	mallocInfo_t mallinfo;

	meminfo(&minfo);

	gettime(&uptime, NULL);

	tid = gettid();
	ret = threadinfo(tid, PH_THREADINFO_CPUTIME, &tinfo);
	if (ret == EOK) {
		cpu = ((double)(tinfo.cpuTime) / (double)uptime) * 100.0f;
	}

	utime = (double)(tinfo.cpuTime - tinfo.sysTime) / 1000000;
	stime = (double)tinfo.sysTime / 1000000;

	mallocInfo(&mallinfo);

	json_object_set_new(root, "proplet_id", json_string(proplet_id));
	json_object_set_new(root, "namespace", json_string(PROPLET_NAMESPACE));
	json_object_set_new(root, "timestamp", json_integer(timestamp));
	json_object_set_new(cmetrics, "user_seconds", json_real(utime));
	json_object_set_new(cmetrics, "system_seconds", json_real(stime));
	json_object_set_new(cmetrics, "percent", json_real(cpu));
	json_object_set_new(mmetrics, "rss_bytes", json_integer(minfo.page.alloc));
	json_object_set_new(mmetrics, "heap_alloc_bytes", json_integer(mallinfo.mapsz - mallinfo.freesz));
	json_object_set_new(mmetrics, "heap_sys_bytes", json_integer(mallinfo.mapsz));
	json_object_set_new(mmetrics, "heap_inuse_bytes", json_integer(mallinfo.mapsz - mallinfo.freesz));
	json_object_set_new(mmetrics, "heap_free_bytes", json_integer(mallinfo.freesz));
	json_object_set_new(mmetrics, "heap_max_alloc_bytes", json_integer(mallinfo.maxalloc));
	json_object_set_new(root, "cpu_metrics", cmetrics);
	json_object_set_new(root, "memory_metrics", mmetrics);

	output = json_dumps(root, JSON_COMPACT);
	json_decref(root);

	return output;
}

char *plt_json_serialize_task_metrics(char *proplet_id, char *task_id, int timestamp, struct proplet_task_metrics *task_metrics)
{
	json_t *root = json_object(), *metrics = json_object(), *aggregated = json_object();
	char *output;

	json_object_set_new(root, "proplet_id", json_string(proplet_id));
	json_object_set_new(root, "task_id", json_string(task_id));
	json_object_set_new(root, "timestamp", json_integer(timestamp));
	json_object_set_new(metrics, "cpu_percent", json_real(task_metrics->snapshot.cpu_percent));
	json_object_set_new(metrics, "memory_bytes", json_integer(task_metrics->snapshot.memory_bytes));
	json_object_set_new(metrics, "memory_percent", json_real(task_metrics->snapshot.memory_percent));
	json_object_set_new(metrics, "disk_read_bytes", json_integer(task_metrics->snapshot.disk_read_bytes));
	json_object_set_new(metrics, "disk_write_bytes", json_integer(task_metrics->snapshot.disk_written_bytes));
	json_object_set_new(metrics, "uptime_seconds", json_integer(task_metrics->snapshot.uptime_seconds));
	json_object_set_new(metrics, "thread_count", json_integer(task_metrics->snapshot.thread_count));
	json_object_set_new(metrics, "file_descriptor_count", json_integer(task_metrics->snapshot.fd_count));
	json_object_set_new(aggregated, "avg_cpu_usage", json_real(task_metrics->aggregated.avg_cpu_usage));
	json_object_set_new(aggregated, "max_cpu_usage", json_real(task_metrics->aggregated.max_cpu_usage));
	json_object_set_new(aggregated, "avg_memory_usage", json_real(task_metrics->aggregated.avg_memory_usage));
	json_object_set_new(aggregated, "max_memory_usage", json_real(task_metrics->aggregated.max_memory_usage));
	json_object_set_new(aggregated, "total_disk_read", json_integer(task_metrics->aggregated.total_disk_read));
	json_object_set_new(aggregated, "total_disk_write", json_integer(task_metrics->aggregated.total_disk_written));
	json_object_set_new(aggregated, "sample_count", json_integer(task_metrics->aggregated.sample_count));
	json_object_set_new(root, "metrics", metrics);
	json_object_set_new(root, "aggregated", aggregated);

	output = json_dumps(root, JSON_COMPACT);
	json_decref(root);

	return output;
}
