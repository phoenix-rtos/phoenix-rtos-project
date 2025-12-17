/*
 * Phoenix-RTOS
 *
 * besmart.energy client for Cognit demo
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <curl/curl.h>
#include <jansson.h>

#include "besmart.h"

#define COGNIT_APP_LOG_TAG "cognit_app/besmart : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_WARN
#include "logger.h"


#define SIGNAL_JSON_MAX_LEN (128)
#define LOGIN_JSON_MAX_LEN  (256)
#define URL_MAX_LEN         (256)
#define HEADER_LINE_MAX_LEN (256)

#define TOKEN_MIN_VALIDITY_SEC (30)


uint64_t getTimeRtcSec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec;
}


/* Adds val to root as a single element array */
static int addAsArrayNew(json_t *root, const char *key, json_t *val)
{
	json_t *arr = json_array();
	if (arr == NULL) {
		return -1;
	}

	if (json_array_append_new(arr, val) < 0) {
		json_decref(arr);
		return -1;
	}

	if (json_object_set_new(root, key, arr) < 0) {
		json_decref(arr);
		return -1;
	}

	return 0;
}


/* Tries adding an element as an array of length 1. */
#define TRY_ADD_JSON_LOC(root__, type__, key__, val__) \
	do { \
		json_t *json__ = json_##type__(val__); \
		if (json__ == NULL) { \
			json_decref(root__); \
			return -1; \
		} \
		if (addAsArrayNew(root__, key__, json__) < 0) { \
			json_decref(json__); \
			json_decref(root__); \
			return -1; \
		} \
	} while (0)


static int prepareSignalJson(char *buf, size_t buflen, const besmart_signalData_t *data)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD_JSON_LOC(root, integer, "time", data->timestamp);
	TRY_ADD_JSON_LOC(root, integer, "origin", 1);

	switch (data->type) {
		case besmart_typeFloat:
			TRY_ADD_JSON_LOC(root, real, "value", data->value.fl);
			TRY_ADD_JSON_LOC(root, string, "type", "DBL");
			break;

		default:
			log_warn("Unknown besmart type %u", data->type);
			json_decref(root);
			return -1;
	}

	size_t ret = json_dumpb(root, buf, buflen, JSON_REAL_PRECISION(4));
	buf[ret] = '\0';
	json_decref(root);

	return 0;
}


static int parseAuthResponse(const char *buf, size_t len, char *tokenBuf, size_t tokenBufSz, uint64_t *expTime)
{
	json_t *root;
	json_error_t err;

	root = json_loads(buf, 0, &err);
	if (root == NULL) {
		log_warn("Parsing auth json failed");
		return -1;
	}

	json_t *token = json_object_get(root, "token");
	if (token != NULL && json_string_length(token) < tokenBufSz - 1) {
		const char *tokenStr = json_string_value(token);
		strncpy(tokenBuf, tokenStr, tokenBufSz);
	}

	json_t *expiration = json_object_get(root, "expiration");
	if (expiration != NULL) {
		double expDouble = json_real_value(expiration);
		*expTime = (uint64_t)expDouble;
	}

	json_decref(root);

	return 0;
}


static int createAuthJson(besmart_ctx_t *ctx, char *buf, size_t buflen)
{
	memset(buf, '\0', buflen);
	json_t *root = json_object();
	json_t *login = json_string(ctx->login);
	json_t *password = json_string(ctx->password);

	json_object_set_new(root, "login", login);
	json_object_set_new(root, "password", password);

	size_t ret = json_dumpb(root, buf, buflen, JSON_REAL_PRECISION(4));
	buf[ret] = '\0';
	json_decref(root);

	return 0;
}


static void clearBuf(besmart_buffer_t *buf)
{
	memset(buf->buf, '\0', sizeof(buf->buf));
	buf->offs = 0;
}


static size_t writeFunctionCb(void *data_content, size_t size, size_t nmemb, void *user_buffer)
{
	size_t realsize = size * nmemb;
	besmart_buffer_t *rxbuf = (besmart_buffer_t *)user_buffer;

	if (sizeof(rxbuf->buf) - rxbuf->offs <= realsize) {
		return 0;
	}

	memcpy(rxbuf->buf + rxbuf->offs, data_content, realsize);
	rxbuf->offs += realsize;
	return realsize;
}


#define TRY_CURL_CLEAN_RAISE(expr__) \
	do { \
		int ret__ = expr__; \
		if (ret__ != CURLE_OK) { \
			curl_slist_free_all(headers); \
			curl_easy_cleanup(curl); \
			return -1; \
		} \
	} while (0)


static int authenticate(besmart_ctx_t *ctx)
{
	if (ctx->tokenExpTime > getTimeRtcSec() + TOKEN_MIN_VALIDITY_SEC) {
		log_debug("Token still valid");
		return 0;
	}

	char loginDataStr[LOGIN_JSON_MAX_LEN];
	int ret = createAuthJson(ctx, loginDataStr, LOGIN_JSON_MAX_LEN);
	if (ret < 0) {
		log_warn("Creating auth payload failed");
		return -1;
	}

	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;

	CURL *curl = curl_easy_init();

	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_URL, "https://api.besmart.energy/api/users/token"));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_POST, 1L));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, loginDataStr));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strnlen(loginDataStr, LOGIN_JSON_MAX_LEN)));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx->rxbuf));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunctionCb));

	clearBuf(&ctx->rxbuf);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		log_warn("curl_easy_perform() failed: %s", curl_easy_strerror(res));
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 200) {
		log_warn("HTTP status: %ld", http_code);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	ctx->rxbuf.buf[ctx->rxbuf.offs] = '\0';
	ret = parseAuthResponse(ctx->rxbuf.buf, ctx->rxbuf.offs, ctx->token, BESMART_MAX_TOKEN_SIZE, &ctx->tokenExpTime);
	if (ret < 0) {
		log_warn("Parsing auth response failed");
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	log_debug("Token: %s", ctx->token);
	log_info("Authenticated successfully!");
	return 0;
}


int besmart_updateSignal(besmart_ctx_t *ctx, const besmart_sensor_t *sensor, const besmart_signalData_t *data)
{
	if (authenticate(ctx) < 0) {
		return -1;
	}

	char signalDataStr[SIGNAL_JSON_MAX_LEN];
	if (prepareSignalJson(signalDataStr, sizeof(signalDataStr), data) < 0) {
		return -1;
	}

	char endpoint[URL_MAX_LEN];
	snprintf(endpoint, sizeof(endpoint) - 1, "https://api.besmart.energy/api/sensors/%d.%d/signals/%d/data",
			sensor->cid, sensor->mid, sensor->moid);

	char auth[HEADER_LINE_MAX_LEN];
	snprintf(auth, sizeof(auth), "Authorization: Bearer %s", ctx->token);

	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;

	CURL *curl = curl_easy_init();

	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, auth);

	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_URL, endpoint));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, signalDataStr));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strnlen(signalDataStr, SIGNAL_JSON_MAX_LEN)));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx->rxbuf));
	TRY_CURL_CLEAN_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunctionCb));

	clearBuf(&ctx->rxbuf);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		log_warn("curl_easy_perform() failed: %s", curl_easy_strerror(res));
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (http_code != 204) {
		log_warn("HTTP status: %ld", http_code);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	log_debug("Updated signal cid: %u, mid: %u, moid: %u", sensor->cid, sensor->mid, sensor->moid);
	return 0;
}


void besmart_setUserData(besmart_ctx_t *ctx, const char *login, const char *password)
{
	ctx->login = login;
	ctx->password = password;
	ctx->tokenExpTime = 0;
	memset(ctx->token, '\0', sizeof(ctx->token));
}


void besmart_init(besmart_ctx_t *ctx)
{
	ctx->login = NULL;
	ctx->password = NULL;
	ctx->tokenExpTime = 0;
	memset(ctx->token, '\0', sizeof(ctx->token));
}
