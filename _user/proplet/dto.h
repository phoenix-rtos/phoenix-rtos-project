/*
 * Phoenix-RTOS
 *
 * MQTT data transfer objects, serialization and deserialization - header
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PH_PROPLET_MQTT_DTO_H
#define _PH_PROPLET_MQTT_DTO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"

struct proplet_task_metrics;

struct proplet_start_request {
	char id[PROPLET_PROPELLER_MAX_ID_SIZE + 1];
	char *name;
	int latent;
	int argc;
	uint32_t *argv;
	size_t codesz;
	uint8_t *code;
};


/*
 * The embedded proplet documentation lies about "name" and "state" fields.
 * As of writing this comment, the payload additionally contains
 * "proplet_id": (uuid/string) and "broadcast" (bool) fields but no "name"
 * and "state". Funnily enough, the documentation instructs the client to
 * silently ignore stop requests lacking "id", "name" or "state" fields.
 */
struct proplet_stop_request {
	char id[PROPLET_PROPELLER_MAX_ID_SIZE + 1];
};


struct proplet_start_request *plt_json_deserialize_start(char *buf, size_t sz, char **imgref);


struct proplet_stop_request *plt_json_deserialize_stop(char *buf, size_t sz);


char *plt_json_serialize_discovery(char *proplet_id, char *description, char *tags, char *location, char *address);


char *plt_json_serialize_system_metrics(char *proplet_id, int timestamp);


char *plt_json_serialize_task_metrics(char *proplet_id, char *task_id, int timestamp, struct proplet_task_metrics *task_metrics);


static inline void plt_obj_dealloc_start_request(struct proplet_start_request *request)
{
	if (request == NULL) {
		return;
	}

	free(request->name);
	free(request->argv);
	free(request->code);
	free(request);
}


#endif /* _PH_PROPLET_MQTT_DTO_H */
