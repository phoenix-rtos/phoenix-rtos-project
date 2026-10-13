/*
 * Phoenix-RTOS
 *
 * Proplet constant definitions
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PS_PROPLET_CONFIG_H
#define _PS_PROPLET_CONFIG_H

#include <assert.h>

#define PROPLET_DEFAULT_LOCATION    "dev-bench"
#define PROPLET_DEFAULT_TAGS        "embedded,phoenix"
#define PROPLET_DEFAULT_DESCRIPTION "Phoenix-RTOS Proplet node"

#define PROPLET_NAMESPACE    "embedded"
#define PROPLET_ENVIRONMENT  "embedded"
#define PROPLET_VERSION      "0.1.0-PROP-PHRTOS"
#define PROPLET_HOST_OS      "Phoenix-RTOS"
#define PROPLET_HOST_ARCH    "x86" /* add define from p-r-build  */
#define PROPLET_WASM_RUNTIME "wamr"

/* These parameters should be carefully adjusted to not cause the peer to disconnect. */
#define PROPLET_MQTT_PROCESS_INTERVAL_USEC 100000
#define PROPLET_MQTT_TIMEOUT               100
#define PROPLET_MQTT_BUFFER_SIZE           1024
#define PROPLET_MQTT_ACK_QUEUE_SIZE        16
#define PROPLET_MQTT_KEEP_ALIVE_SECONDS    120
#define PROPLET_MQTT_MAX_TOPIC_LENGTH      128 /* Adjust this if there is a topic longer that current value */
#define PROPLET_MQTT_QOS                   MQTTQoS2
#define PROPLET_READ_TIMEOUT_USEC          200000

// _Static_assert(PROPLET_MQTT_PROCESS_INTERVAL_USEC % 100000 == 0, "Loop interval has to be divisible");

/*
 * Length of a UUID string, UUID are currently used in Propeller for object IDs.
 */
#define PROPLET_PROPELLER_MAX_ID_SIZE 36

#define PROPLET_MAX_INLINE_PAYLOAD_SIZE 1024

#define PROPLET_MAX_WASM_TASKS      10
#define PROPLET_MAX_MONITORED_TASKS 4

#define PROPLET_WASM_STACK_SIZE  4096 * 4
#define PROPLET_WASM_HEAP_SIZE   4096 * 4
#define PROPLET_WASM_MAX_THREADS 1

#define PROPLET_WASM_ERROR_BUFFER_SIZE 1024

#endif /* _PS_PROPLET_CONFIG_H */
