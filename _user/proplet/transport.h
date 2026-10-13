/*
 * Phoenix-RTOS
 *
 * MQTT transport layer implementation header
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PS_PROPLET_TRANSPORT_H
#define _PS_PROPLET_TRANSPORT_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <core_mqtt.h>

#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

struct NetworkContext {
	/* clang-format off */
	enum { transport_raw, transport_tls } kind;
	/* clang-format on */
	union {
		int rawfd;
		struct {
			mbedtls_net_context net;
			mbedtls_ssl_context context;
			mbedtls_ssl_config config;
			mbedtls_x509_crt ca;
			mbedtls_x509_crt client;
			mbedtls_pk_context pk;
			mbedtls_entropy_context entropy;
			mbedtls_ctr_drbg_context rng;
		} ssl;
	};
};

static inline void transport_getRemoteIp(int sock, char *output, size_t outlen)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	getsockname(sock, (struct sockaddr *)&addr, &len);
	inet_ntop(AF_INET, &addr.sin_addr, output, outlen); /* TODO: what about v6... */
}


int32_t transport_send_handler(NetworkContext_t *ctx, const void *buf, size_t sz);


int32_t transport_recv_handler(NetworkContext_t *ctx, void *buf, size_t sz);


void transport_connect(NetworkContext_t *ctx, char *host, char *port, bool secure, bool verify,
		char *cabundle, char *clientcert, char *clientkey);


void transport_destroy(NetworkContext_t *ctx);


#endif /* _PS_PROPLET_TRANSPORT_H */
