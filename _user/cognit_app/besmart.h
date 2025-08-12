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

#ifndef COGNIT_APP_BESMART_H
#define COGNIT_APP_BESMART_H

#include <stddef.h>
#include <stdint.h>


/* NOTE: libcurl is used for HTTPS protocol */

#define BESMART_HOSTNAME       "api.besmart.energy"
#define BESMART_MAX_TOKEN_SIZE (128)


typedef enum {
	besmart_originMeasuredData = 1,
	besmart_originPredictedData = 2,
	besmart_originCalculatedData = 3,
} besmart_dataOrigin_t;


typedef enum {
	besmart_typeFloat,
} besmart_dataType_t;


typedef struct {
	uint16_t cid;
	uint16_t mid;
	uint16_t moid;
} besmart_sensor_t;


typedef struct {
	uint64_t timestamp; /* ms */
	union {
		double fl;
	} value;
	besmart_dataType_t type;
	besmart_dataOrigin_t origin;
} besmart_signalData_t;


typedef struct {
	char buf[2048];
	size_t offs;
} besmart_buffer_t;


typedef struct {
	const char *login;
	const char *password;

	uint64_t tokenExpTime;
	char token[BESMART_MAX_TOKEN_SIZE + 1];
	besmart_buffer_t rxbuf;
} besmart_ctx_t;


int besmart_updateSignal(besmart_ctx_t *ctx, const besmart_sensor_t *sensor, const besmart_signalData_t *data);


/* Login and password are not copied to library internal buffers. Library uses pointers to strings held by the user. */
void besmart_setUserData(besmart_ctx_t *ctx, const char *login, const char *password);


void besmart_init(besmart_ctx_t *ctx);


#endif /* COGNIT_APP_BESMART_H */
