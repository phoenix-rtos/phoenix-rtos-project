#include <stdio.h>
#include <cognit/cognit_http.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <cognit/device_runtime.h>
#include <unistd.h>
#include <cognit/cognit_http.h>
#include <cognit/cognit_encoding.h>
#include <cognit/logger.h>
// #include <ip_utils.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>

// Function to be offloaded.
char *fc_str = "def my_calc(operation, param1, param2):\n"
			   "    if operation == \"sum\":\n"
			   "        result = param1 + param2\n"
			   "    elif operation == \"multiply\":\n"
			   "        result = param1 * param2\n"
			   "    else:\n"
			   "        result = 0.0\n"
			   "    return result\n";
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

size_t handle_response_data_cb(void *data_content, size_t size, size_t nmemb, void *user_buffer)
{
	size_t realsize = size * nmemb;
	http_response_t *response = (http_response_t *)user_buffer;

	if (response->size + realsize >= sizeof(response->ui8_response_data_buffer)) {
		COGNIT_LOG_ERROR("Response buffer too small");
		return 0;
	}

	memcpy(&(response->ui8_response_data_buffer[response->size]), data_content, realsize);
	response->size += realsize;
	response->ui8_response_data_buffer[response->size] = '\0';

	return realsize;
}

int8_t my_http_send_req_cb(const char *c_buffer, size_t size, http_config_t *config)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	long http_code = 0;
	struct curl_slist *headers = NULL;
	memset(&config->t_http_response.ui8_response_data_buffer, 0, sizeof(config->t_http_response.ui8_response_data_buffer));
	config->t_http_response.size = 0;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		// Set the request header
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		// headers = curl_slist_append(headers, "charset: utf-8");

		if (config->c_token != NULL) {
			char token_header[MAX_TOKEN_LENGTH] = "token: ";
			strcat(token_header, config->c_token);
			headers = curl_slist_append(headers, token_header);
		}

		if (curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) != CURLE_OK
				// Configure URL and payload
				|| curl_easy_setopt(curl, CURLOPT_URL, config->c_url) != CURLE_OK
				// Set the callback function to handle the response data
				|| curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&config->t_http_response) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response_data_cb) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config->ui32_timeout_ms) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L) != CURLE_OK) {
			COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt() failed");
			return -1;
		}

		// Find '[' or ']' in the URL to determine the IP version
		// TODO: fix ip_utils to obtain http://[2001:67c:22b8:1::d]:8000/v1/faas/execute-sync
		// as IP_V6
		if (strchr(config->c_url, '[') != NULL && strchr(config->c_url, ']') != NULL) {
			if (curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6) != CURLE_OK) {
				COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt()->IPRESOLVE_V6 failed");
				return -1;
			}
		}

		if (strcmp(config->c_method, HTTP_METHOD_GET) == 0) {
			if (curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_USERNAME, config->c_username) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_PASSWORD, config->c_password) != CURLE_OK) {
				COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt()->get() failed");
				return -1;
			}
		}
		else if (strcmp(config->c_method, HTTP_METHOD_POST) == 0) {
			if (curl_easy_setopt(curl, CURLOPT_POST, 1L) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST") != CURLE_OK || curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_POSTFIELDS, c_buffer) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_USERNAME, config->c_username) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_PASSWORD, config->c_password) != CURLE_OK) {
				COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt()->post() failed");
				return -1;
			}
		}
		else if (strcmp(config->c_method, HTTP_METHOD_PUT) == 0) {
			if (curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT") != CURLE_OK || curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_POSTFIELDS, c_buffer) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_USERNAME, config->c_username) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_PASSWORD, config->c_password) != CURLE_OK) {
				COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt()->put() failed");
				return -1;
			}
		}
		else if (strcmp(config->c_method, HTTP_METHOD_DELETE) == 0) {
			if (curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE") != CURLE_OK || curl_easy_setopt(curl, CURLOPT_USERNAME, config->c_username) != CURLE_OK || curl_easy_setopt(curl, CURLOPT_PASSWORD, config->c_password) != CURLE_OK) {
				COGNIT_LOG_ERROR("[http_send_req_cb] curl_easy_setopt()->post() failed");
				return -1;
			}
		}
		else {
			COGNIT_LOG_ERROR("[http_send_req_cb] Invalid HTTP method");
			return -1;
		}

		// Make the request
		res = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		COGNIT_LOG_INFO("HTTP err code %ld ", http_code);

		// Check errors
		if (res != CURLE_OK) {
			http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			COGNIT_LOG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
			COGNIT_LOG_ERROR("HTTP err code %ld ", http_code);
		}

		// Clean and close CURL session
		curl_easy_cleanup(curl);
	}

	config->t_http_response.l_http_code = http_code;

	// Clean global curl configuration
	curl_global_cleanup();
	curl_slist_free_all(headers);

	return (res == CURLE_OK) ? 0 : -1;
}

cognit_config_t t_config = {
	.cognit_frontend_endpoint = "",
	.cognit_frontend_usr = "",  // Put your username here.
	.cognit_frontend_pwd = "",  // Put your password here.
};

// Set your own App requirements.
scheduling_t app_reqs = {
	.flavour = "Energy",                 // Put a Flavour that your username is allowed to use.
	.max_latency = 100,                  // Max latency required in miliseconds.
	.max_function_execution_time = 3.5,  // Max execution time required in seconds.
	.min_renewable = 85,                 // Minimal renewable energy resources required in percentage.
	.geolocation = {
		.latitude = 40.4168f,
		.longitude = -3.7038f }
};

// Set your new App requirements.
scheduling_t new_reqs = {
	.flavour = "Energy",                 // Put a Flavour that your username is allowed to use.
	.max_latency = 80,                   // Max latency required in miliseconds.
	.max_function_execution_time = 8.5,  // Max execution time required in seconds.
	.min_renewable = 50,                 // Minimal renewable energy resources required in percentage.
	.geolocation = {
		.latitude = 40.4168f,
		.longitude = -3.7038f }
};

int main(int argc, char const *argv[])
{
	device_runtime_t t_my_device_runtime;
	faas_t t_faas;
	double *exec_response;
	e_status_code_t ret;

	ret = device_runtime_init(&t_my_device_runtime, t_config, app_reqs, &t_faas);
	if (ret != E_ST_CODE_SUCCESS) {
		COGNIT_LOG_ERROR("Could not initialize device runtime");
		return EXIT_FAILURE;
	}

	addFC(&t_faas, fc_str);

	addSTRINGParam(&t_faas, "sum");
	addINT32Var(&t_faas, 8);
	addFLOATVar(&t_faas, 3.5);

	ret = device_runtime_call(&t_my_device_runtime, &t_faas, new_reqs, (void **)&exec_response);

	if (ret == E_ST_CODE_SUCCESS) {
		COGNIT_LOG_INFO("Result: %f", *exec_response);

		// Free response pointer
		free(exec_response);
	}
	else {
		COGNIT_LOG_ERROR("Error offloading function");
	}
	return 0;
}
