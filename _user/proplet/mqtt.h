/*
 * Phoenix-RTOS
 *
 * coreMQTT wrapper
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PS_PROPLET_MQTT_H
#define _PS_PROPLET_MQTT_H

#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>

#include <core_mqtt.h>

#include "taskman.h"

#define PROPLET_OFFLINE_MESSAGE_FMT "{\"status\":\"offline\",\"proplet_id\":\"%s\",\"namespace\":\"embedded\"}"
#define PROPLET_ALIVE_MESSAGE_FMT   "{\"status\":\"alive\",\"proplet_id\":\"%s\",\"namespace\":\"embedded\"}"
#define PROPLET_RESULTS_MESSAGE_FMT "{\"task_id\":\"%36s\",\"results\":\"%d\\n\",\"error\":\"%s\"}"


struct proplet_context {
	char *proplet_id;
	char *client_key;
	char *domain_id;
	char *channel_id;
	char *tags;
	char *location;
	char *description;
};


/*
 * Initializes the coreMQTT library and proplet context
 */
MQTTStatus_t plt_mqtt_init(struct proplet_context *ctx);


/* */
MQTTStatus_t plt_mqtt_connect(char *host, char *port, bool secure, bool verify,
		char *cabundle, char *clientcert, char *clientkey);


MQTTStatus_t plt_mqtt_subscribe(void);


MQTTStatus_t plt_mqtt_publish_discovery(void);


MQTTStatus_t plt_mqtt_publish_alive(void);


MQTTStatus_t plt_mqtt_publish_task_metrics(char *task_id, struct proplet_task_metrics *metrics);


MQTTStatus_t plt_mqtt_publish_system_metrics(void);


MQTTStatus_t plt_mqtt_publish_image_request(char *image);


MQTTStatus_t plt_mqtt_process(void);


MQTTStatus_t plt_mqtt_publish_results(char *id, int results, char *error);


MQTTStatus_t plt_mqtt_publish_image_request(char *image);


void plt_mqtt_destroy(void);


#endif /* _PS_PROPLET_MQTT_H */
