/*
 * Phoenix-RTOS
 *
 * MQTT transport layer implementation
 *
 * Copyright 2026 Phoenix Systems
 * Author: Michal Lach
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/tcp.h>

#include <psa/crypto.h>

#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/platform.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>

#include "config.h"
#include "log.h"
#include "transport.h"


char __mbedtls_error_buffer[1024];


static inline int tcp_send(NetworkContext_t *ctx, const void *buf, size_t sz)
{
	return write(ctx->rawfd, buf, sz);
}

static inline int tcp_recv(NetworkContext_t *ctx, void *buf, size_t sz)
{
	return read(ctx->rawfd, buf, sz);
}

static inline int tls_send(NetworkContext_t *ctx, const void *buf, size_t sz)
{
	int ret = mbedtls_ssl_write(&ctx->ssl.context, buf, sz);

	if (ret < 0 && ret != MBEDTLS_ERR_SSL_TIMEOUT) {
		mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
		log_error("tls_send() failed with: %s", __mbedtls_error_buffer);
	}

	return ret;
}

static inline int tls_recv(NetworkContext_t *ctx, void *buf, size_t sz)
{
	int ret = mbedtls_ssl_read(&ctx->ssl.context, buf, sz);

	if (ret < 0 && ret != MBEDTLS_ERR_SSL_TIMEOUT) {
		mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
		log_error("tls_send() failed with: %s", __mbedtls_error_buffer);
	}

	return ret;
}

int32_t transport_send_handler(NetworkContext_t *ctx, const void *buf, size_t sz)
{
	switch (ctx->kind) {
		case transport_raw:
			return tcp_send(ctx, buf, sz);
		case transport_tls:
			return tls_send(ctx, buf, sz);
		default:
			assert(false);
	}

	__builtin_unreachable();
}


int32_t transport_recv_handler(NetworkContext_t *ctx, void *buf, size_t sz)
{
	switch (ctx->kind) {
		case transport_raw:
			return tcp_recv(ctx, buf, sz);
		case transport_tls:
			return tls_recv(ctx, buf, sz);
		default:
			assert(false);
	}

	__builtin_unreachable();
}


void transport_destroy(NetworkContext_t *ctx)
{
	switch (ctx->kind) {
		case transport_tls:
			mbedtls_net_free(&ctx->ssl.net);
			mbedtls_ssl_free(&ctx->ssl.context);
			mbedtls_ssl_config_free(&ctx->ssl.config);
			mbedtls_x509_crt_free(&ctx->ssl.ca);
			mbedtls_x509_crt_free(&ctx->ssl.client);
			mbedtls_pk_free(&ctx->ssl.pk);
			mbedtls_ctr_drbg_free(&ctx->ssl.rng);
			mbedtls_entropy_free(&ctx->ssl.entropy);
			/* fallthrough */
		case transport_raw:
			close(ctx->rawfd);
			break;
		default:
			assert(false);
	}
}


void transport_connect(NetworkContext_t *ctx, char *host, char *port, bool secure,
		bool verify, char *capath, char *certpath, char *keypath)
{
	int sock = -1, ret, val = 1;
	struct addrinfo *info, hints;
	struct timeval tv;
	mbedtls_net_context net_context;
	mbedtls_ssl_context ssl_context;
	mbedtls_ctr_drbg_context rng_context;
	mbedtls_entropy_context entropy;
	mbedtls_ssl_config config;
	mbedtls_x509_crt ca, cc;
	mbedtls_pk_context pk;

	if (secure) {
		psa_crypto_init();

		mbedtls_ctr_drbg_init(&rng_context);
		mbedtls_entropy_init(&entropy);

		ret = mbedtls_ctr_drbg_seed(&rng_context, mbedtls_entropy_func, &entropy, NULL, 0);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not seed RNG entropy: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_x509_crt_init(&ca);
		ret = mbedtls_x509_crt_parse_file(&ca, capath);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not load CA bundle: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_x509_crt_init(&cc);
		ret = mbedtls_x509_crt_parse_file(&cc, certpath);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not load client certificate: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_pk_init(&pk);
		ret = mbedtls_pk_parse_keyfile(&pk, keypath, NULL);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not parse certificate key: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_net_init(&net_context);
		ret = mbedtls_net_connect(&net_context, host, port, MBEDTLS_NET_PROTO_TCP);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not connect: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_ssl_init(&ssl_context);
		mbedtls_ssl_config_init(&config);

		ret = mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not configure TLS: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_ssl_conf_authmode(&config, verify ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
		mbedtls_ssl_conf_own_cert(&config, &cc, &pk);
		mbedtls_ssl_conf_read_timeout(&config, PROPLET_READ_TIMEOUT_USEC / 1000 /* millis */);
		mbedtls_ssl_conf_ca_chain(&config, &ca, NULL);
		mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, &rng_context);

		ret = mbedtls_ssl_setup(&ssl_context, &config);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not setup TLS: %s\n", __mbedtls_error_buffer);
		}

		ret = mbedtls_ssl_set_hostname(&ssl_context, host);
		if (ret < 0) {
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not set TLS remote hostname: %s\n", __mbedtls_error_buffer);
		}

		mbedtls_ssl_set_bio(&ssl_context, &net_context, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

		do {
			ret = mbedtls_ssl_handshake(&ssl_context);
			if (ret != 0 && ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				if (PLT_LOGGING_VERBOSE) {
					ret = mbedtls_ssl_get_verify_result(&ssl_context);
					mbedtls_x509_crt_verify_info((char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer), "  ! ", ret);
					fail("mbedtls_ssl_get_verify_info(): %s", __mbedtls_error_buffer);
				}
				else {
					mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
					fail("could not perform a TLS handshake: %s\n", __mbedtls_error_buffer);
				}
			}
		} while (ret != 0);

		ret = mbedtls_ssl_get_verify_result(&ssl_context);
		if (ret < 0) {
			mbedtls_x509_crt_verify_info((char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer), "  ! ", ret);
			fail("mbedtls_ssl_get_verify_info(): %s", __mbedtls_error_buffer);
			mbedtls_strerror(ret, (char *)&__mbedtls_error_buffer, sizeof(__mbedtls_error_buffer));
			fail("could not verify TLS handshake: %s\n", __mbedtls_error_buffer);
		}

		ctx->kind = transport_tls;
		ctx->ssl.net = net_context;
		ctx->ssl.context = ssl_context;
		ctx->ssl.ca = ca;
		ctx->ssl.client = cc;
		ctx->ssl.pk = pk;
		ctx->ssl.entropy = entropy;
		ctx->ssl.rng = rng_context;
		/* @FIXME(michal.lach): mbedtls_net_context is hypothetically opaque */
		ctx->rawfd = net_context.fd;
	}
	else {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; /* @TODO(michal.lach): no support for v6 */
		hints.ai_socktype = SOCK_STREAM;

		ret = getaddrinfo(host, port, &hints, &info);
		if (ret != 0) {
			fail("getaddrinfo() failed with: %s", strerror(ret));
		}

		sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		if (sock == -1) {
			fail("could not open a socket: %s", strerror(ret));
		}

		tv.tv_sec = 0;
		tv.tv_usec = PROPLET_READ_TIMEOUT_USEC;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val));

		ret = connect(sock, info->ai_addr, info->ai_addrlen);
		if (ret == -1) {
			fail("could not connect to host: %s", strerror(ret));
		}

		freeaddrinfo(info);

		ctx->kind = transport_raw;
		ctx->rawfd = sock;
	}
}
