/*
 * Phoenix-RTOS
 *
 * Callbacks for the use of cognit lib
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>

#include <curl/curl.h>

#include <cognit/cognit_http.h>
#include <cognit/cognit_encoding.h>
#include <cognit/device_runtime.h>
#include <cognit/cognit_http.h>


#define COGNIT_APP_LOG_TAG "cognit_app/lib_cbs : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_INFO
#include "logger.h"


int my_base64_encode_cb(unsigned char str_b64_buff[], size_t buff_len, size_t *base64_len, char str[], int str_len)
{
	return mbedtls_base64_encode(str_b64_buff, buff_len, base64_len, (uint8_t *)str, str_len);
}


int my_base64_decode_cb(char decoded_buff[], size_t buff_size, size_t *decoded_len, const unsigned char *str, size_t str_len)
{
	return mbedtls_base64_decode((uint8_t *)decoded_buff, buff_size, decoded_len, str, str_len);
}


int my_hash_cb(const unsigned char *str, size_t str_len, unsigned char hash[])
{
	return mbedtls_sha256_ret(str, str_len, hash, 0);
}


static size_t writeFunctionCb(void *data_content, size_t size, size_t nmemb, void *user_buffer)
{
	size_t realsize = size * nmemb;
	http_response_t *response = (http_response_t *)user_buffer;

	if (response->size + realsize >= sizeof(response->ui8_response_data_buffer)) {
		log_error("Response buffer too small");
		return 0;
	}

	memcpy(&(response->ui8_response_data_buffer[response->size]), data_content, realsize);
	response->size += realsize;
	response->ui8_response_data_buffer[response->size] = '\0';

	return realsize;
}


#define TRY_CURL_RAISE(expr__) \
	do { \
		int ret__ = expr__; \
		if (ret__ != CURLE_OK) { \
			log_warn("Curl error line: %d", __LINE__); \
			return -1; \
		} \
	} while (0)


typedef enum {
	httpMethodGet,
	httpMethodPost,
	httpMethodPut,
	httpMethodDelete,
	httpMethodUnknown,
} httpMethod_t;


static httpMethod_t getHttpMethod(const char *str)
{
	if (strcmp(str, HTTP_METHOD_GET) == 0) {
		return httpMethodGet;
	}
	else if (strcmp(str, HTTP_METHOD_POST) == 0) {
		return httpMethodPost;
	}
	else if (strcmp(str, HTTP_METHOD_PUT) == 0) {
		return httpMethodPut;
	}
	else if (strcmp(str, HTTP_METHOD_DELETE) == 0) {
		return httpMethodDelete;
	}
	else {
		return httpMethodUnknown;
	}
}


static int buildCurlReq(const char *c_buffer, size_t size, http_config_t *config, CURL *curl, struct curl_slist *headers)
{
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_URL, config->c_url));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&config->t_http_response));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunctionCb));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->ui32_timeout_ms / 1000));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L));

	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_USERNAME, config->c_username));
	TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_PASSWORD, config->c_password));

	httpMethod_t httpMethod = getHttpMethod(config->c_method);

	switch (httpMethod) {
		case httpMethodGet:
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L));
			break;

		case httpMethodPost:
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_POST, 1L));
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST"));
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size));
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, c_buffer));
			break;

		case httpMethodPut:
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"));
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size));
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, c_buffer));
			break;

		case httpMethodDelete:
			TRY_CURL_RAISE(curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"));
			break;

		default:
			return -1;
	}

	return 0;
}


int8_t my_http_send_req_cb(const char *c_buffer, size_t size, http_config_t *config)
{
	memset(&config->t_http_response.ui8_response_data_buffer, 0, sizeof(config->t_http_response.ui8_response_data_buffer));
	config->t_http_response.size = 0;

	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;

	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		log_error("curl_easy_init failed");
		return -1;
	}

	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	if (config->c_token != NULL) {
		char token_header[MAX_TOKEN_LENGTH + 1];
		memset(token_header, '\0', sizeof(token_header));
		snprintf(token_header, MAX_TOKEN_LENGTH, "token: %s", config->c_token);
		headers = curl_slist_append(headers, token_header);
	}

	if (buildCurlReq(c_buffer, size, config, curl, headers) < 0) {
		log_warn("buildCurlReq failed");
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return -1;
	}

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		log_warn("curl_easy_perform() failed: %s", curl_easy_strerror(res));
	}

	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	config->t_http_response.l_http_code = http_code;
	log_debug("HTTP err code %ld ", http_code);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK) ? 0 : -1;
}
