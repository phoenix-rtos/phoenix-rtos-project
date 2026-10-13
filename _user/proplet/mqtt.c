/*
 * Phoenix-RTOS
 *
 * coreMQTT wrapper implementation
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include <time.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <jansson.h>

#include "transport.h"
#include "mqtt.h"
#include "base64.h"
#include "log.h"
#include "dto.h"
#include "taskman.h"


enum proplet_topic {
	topic_publish_topics = 0,
	topic_control_create = topic_publish_topics,
	topic_control_alive,
	topic_control_metrics,
	topic_control_task_metrics,
	topic_control_results,
	topic_subscribe_topics,
	topic_control_start = topic_subscribe_topics,
	topic_control_stop,
	topic_max,
};


char *__plt_topic_fmts[topic_max] = {
	"m/%s/c/%s/control/proplet/create",
	"m/%s/c/%s/control/proplet/alive",
	"m/%s/c/%s/control/proplet/metrics",
	"m/%s/c/%s/control/proplet/task_metrics",
	"m/%s/c/%s/control/proplet/results",
	"m/%s/c/%s/control/manager/start",
	"m/%s/c/%s/control/manager/stop",
};


struct {
	MQTTContext_t mqtt;
	NetworkContext_t net;
	pthread_mutex_t mtx;

	char net_buffer[PROPLET_MQTT_BUFFER_SIZE];
	MQTTPubAckInfo_t out_acks[PROPLET_MQTT_ACK_QUEUE_SIZE];
	MQTTPubAckInfo_t in_acks[PROPLET_MQTT_ACK_QUEUE_SIZE];

	struct proplet_context proplet_context;

	char topics[topic_max][PROPLET_MQTT_MAX_TOPIC_LENGTH];
	char alive_message[PROPLET_MQTT_MAX_TOPIC_LENGTH];
	char offline_message[PROPLET_MQTT_MAX_TOPIC_LENGTH];
} mqtt_context;


/* called with held mutex on MQTTContext_t * */
static void __plt_mqtt_handler(MQTTContext_t *ctx, MQTTPacketInfo_t *packet_info,
		MQTTDeserializedInfo_t *deserialized_info)
{
	MQTTPublishInfo_t *info = deserialized_info->pPublishInfo;
	int ret = 0;

	if ((packet_info->type & 0xF0) == MQTT_PACKET_TYPE_PUBLISH) {
		log_info("Received packet on topic %s", info->pTopicName);
		log_debug("got: %.*s", (int)info->payloadLength, (char *)info->pPayload);

		if (strcmp(info->pTopicName, mqtt_context.topics[topic_control_start]) == 0) {
			struct proplet_start_request *request;
			char *ref = NULL;

			request = plt_json_deserialize_start((char *)info->pPayload, info->payloadLength, &ref);
			if (request == NULL) {
				log_error("failed to deserialize control/manager/start message");
				return;
			}

			/*
			 * plt_taskman_register_task() steals the reference to request and
			 * manages it until end of execution
			 */
			plt_taskman_register_task(request);
		}
		else if (strcmp(info->pTopicName, mqtt_context.topics[topic_control_stop]) == 0) {
			struct proplet_stop_request *request;

			request = plt_json_deserialize_stop((char *)info->pPayload, info->payloadLength);
			if (request == NULL) {
				log_error("failed to deserialize control/manager/stop message");
				return;
			}

			ret = plt_taskman_stop_task(request->id);
			if (ret != 0) {
				log_error("unable to stop task: %s", strerror(-ret));
				return;
			}

			free(request);
		}
		else {
			fail("Got packet on an unexpected topic, quitting");
		}
	}
}


static uint32_t get_timestamp(void)
{
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}


static MQTTStatus_t plt_mqtt_setup(void)
{
	MQTTStatus_t status;
	TransportInterface_t transport = {
		.pNetworkContext = &mqtt_context.net,
		.send = transport_send_handler,
		.recv = transport_recv_handler,
	};
	MQTTFixedBuffer_t mqtt_buffer = {
		.pBuffer = (uint8_t *)mqtt_context.net_buffer,
		.size = sizeof(mqtt_context.net_buffer),
	};

	status = MQTT_Init(
			&mqtt_context.mqtt,
			&transport,
			get_timestamp,
			__plt_mqtt_handler,
			&mqtt_buffer);

	if (status != MQTTSuccess) {
		return status;
	}

	return MQTT_InitStatefulQoS(
			&mqtt_context.mqtt,
			mqtt_context.out_acks,
			PROPLET_MQTT_ACK_QUEUE_SIZE,
			mqtt_context.in_acks,
			PROPLET_MQTT_ACK_QUEUE_SIZE);
}


static void plt_mqtt_propagate_topics(struct proplet_context *ctx)
{
	int ret = 0;

	ret = snprintf(NULL, 0, PROPLET_OFFLINE_MESSAGE_FMT, ctx->proplet_id);
	if (ret < 0 || ret > PROPLET_MQTT_MAX_TOPIC_LENGTH) {
		fail("could not format MQTT topics: %s", strerror(ret));
	}

	memset(mqtt_context.offline_message, 0, PROPLET_MQTT_MAX_TOPIC_LENGTH);
	snprintf(mqtt_context.offline_message, ret + 1, PROPLET_OFFLINE_MESSAGE_FMT, ctx->proplet_id);

	ret = snprintf(NULL, 0, PROPLET_ALIVE_MESSAGE_FMT, ctx->proplet_id);
	if (ret < 0 || ret > PROPLET_MQTT_MAX_TOPIC_LENGTH) {
		fail("could not format MQTT topics: %s", strerror(ret));
	}

	memset(mqtt_context.alive_message, 0, PROPLET_MQTT_MAX_TOPIC_LENGTH);
	snprintf(mqtt_context.alive_message, ret + 1, PROPLET_ALIVE_MESSAGE_FMT, ctx->proplet_id);

	for (int i = 0; i < sizeof(__plt_topic_fmts) / sizeof(*__plt_topic_fmts); i++) {
		ret = snprintf(NULL, 0, __plt_topic_fmts[i], ctx->domain_id, ctx->channel_id);
		if (ret < 0 || ret > PROPLET_MQTT_MAX_TOPIC_LENGTH) {
			fail("could not format MQTT topics: %s", strerror(ret));
		}

		memset(mqtt_context.topics[i], 0, PROPLET_MQTT_MAX_TOPIC_LENGTH);
		snprintf(mqtt_context.topics[i], ret + 1, __plt_topic_fmts[i], ctx->domain_id, ctx->channel_id);
	}
}


MQTTStatus_t plt_mqtt_init(struct proplet_context *proplet_context)
{
	pthread_mutex_init(&mqtt_context.mtx, NULL);
	memcpy(&mqtt_context.proplet_context, proplet_context, sizeof(*proplet_context));
	plt_mqtt_propagate_topics(proplet_context);

	return plt_mqtt_setup();
}


MQTTStatus_t plt_mqtt_connect(char *host, char *port, bool secure, bool verify,
		char *cabundle, char *clientcert, char *clientkey)
{
	bool parent;
	MQTTConnectInfo_t info = {
		.cleanSession = true,
		.keepAliveSeconds = PROPLET_MQTT_KEEP_ALIVE_SECONDS,
		.pClientIdentifier = mqtt_context.proplet_context.proplet_id,
		.clientIdentifierLength = strlen(mqtt_context.proplet_context.proplet_id),
		.pUserName = mqtt_context.proplet_context.proplet_id,
		.userNameLength = strlen(mqtt_context.proplet_context.proplet_id),
		.pPassword = mqtt_context.proplet_context.client_key,
		.passwordLength = strlen(mqtt_context.proplet_context.client_key),
	};
	MQTTPublishInfo_t will = {
		.qos = MQTTQoS0, /* This has to stay QoS0 */
		.pTopicName = mqtt_context.topics[topic_control_alive],
		.topicNameLength = strlen(mqtt_context.topics[topic_control_alive]),
		.pPayload = mqtt_context.offline_message,
		.payloadLength = strlen(mqtt_context.offline_message),
	};

	transport_connect(&mqtt_context.net, host, port, secure, verify, cabundle, clientcert, clientkey);

	return MQTT_Connect(&mqtt_context.mqtt, &info, &will, PROPLET_MQTT_TIMEOUT, &parent);
}


MQTTStatus_t plt_mqtt_subscribe(void)
{
	uint16_t id;
	MQTTStatus_t status;
	MQTTSubscribeInfo_t info[] = {
		{
			.qos = PROPLET_MQTT_QOS,
			.pTopicFilter = mqtt_context.topics[topic_control_start],
			.topicFilterLength = strlen(mqtt_context.topics[topic_control_start]),
		},
		{
			.qos = PROPLET_MQTT_QOS,
			.pTopicFilter = mqtt_context.topics[topic_control_stop],
			.topicFilterLength = strlen(mqtt_context.topics[topic_control_stop]),
		},
	};

	pthread_mutex_lock(&mqtt_context.mtx);
	id = MQTT_GetPacketId(&mqtt_context.mqtt);
	status = MQTT_Subscribe(&mqtt_context.mqtt, info, sizeof(info) / sizeof(*info), id);
	pthread_mutex_unlock(&mqtt_context.mtx);

	return status;
}


static MQTTStatus_t publish_message(enum proplet_topic topic, char *payload, size_t sz)
{
	uint16_t id;
	MQTTStatus_t status;
	MQTTPublishInfo_t info = {
		.qos = PROPLET_MQTT_QOS,
		.pTopicName = mqtt_context.topics[topic],
		.topicNameLength = strlen(mqtt_context.topics[topic]),
		.pPayload = payload,
		.payloadLength = sz,
		.dup = false,
		.retain = false,
	};

	log_debug("(%s) publishing %s", mqtt_context.topics[topic], payload);

	pthread_mutex_lock(&mqtt_context.mtx);
	id = MQTT_GetPacketId(&mqtt_context.mqtt);
	status = MQTT_Publish(&mqtt_context.mqtt, &info, id);
	pthread_mutex_unlock(&mqtt_context.mtx);

	return status;
}


MQTTStatus_t plt_mqtt_publish_discovery(void)
{
	char addr[17], *output;
	MQTTStatus_t status;

	memset(addr, 0, sizeof(addr));
	transport_getRemoteIp(mqtt_context.net.rawfd, addr, 16);

	output = plt_json_serialize_discovery(
			mqtt_context.proplet_context.proplet_id,
			mqtt_context.proplet_context.description,
			mqtt_context.proplet_context.tags,
			mqtt_context.proplet_context.location,
			addr);

	status = publish_message(topic_control_create, output, strlen(output));
	free(output);

	return status;
}


MQTTStatus_t plt_mqtt_publish_alive(void)
{
	return publish_message(topic_control_alive, mqtt_context.alive_message, strlen(mqtt_context.alive_message));
}


MQTTStatus_t plt_mqtt_publish_task_metrics(char *task_id, struct proplet_task_metrics *metrics)
{
	MQTTStatus_t status;
	char *output;

	output = plt_json_serialize_task_metrics(mqtt_context.proplet_context.proplet_id, task_id, get_timestamp(), metrics);
	status = publish_message(topic_control_task_metrics, output, strlen(output));
	free(output);

	return status;
}


MQTTStatus_t plt_mqtt_publish_system_metrics(void)
{
	MQTTStatus_t status;
	char *output;

	output = plt_json_serialize_system_metrics(mqtt_context.proplet_context.proplet_id, get_timestamp());
	status = publish_message(topic_control_metrics, output, strlen(output));
	free(output);

	return status;
}


MQTTStatus_t plt_mqtt_publish_results(char *id, int results, char *error)
{
	int ret = 0;
	char *output = NULL;
	MQTTStatus_t status;

	ret = snprintf(NULL, 0, PROPLET_RESULTS_MESSAGE_FMT, id, results, error == NULL ? "null" : error);
	output = calloc(ret + 1, 1);
	snprintf(output, ret + 1, PROPLET_RESULTS_MESSAGE_FMT, id, results, error == NULL ? "null" : error);
	status = publish_message(topic_control_results, output, strlen(output));
	free(output);

	return status;
}


MQTTStatus_t plt_mqtt_process(void)
{
	MQTTStatus_t status;

	pthread_mutex_lock(&mqtt_context.mtx);
	status = MQTT_ProcessLoop(&mqtt_context.mqtt);
	pthread_mutex_unlock(&mqtt_context.mtx);

	return status;
}


void plt_mqtt_destroy(void)
{
	transport_destroy(&mqtt_context.net);
	pthread_mutex_destroy(&mqtt_context.mtx);
}
