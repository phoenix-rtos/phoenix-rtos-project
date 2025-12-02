/*
 * Phoenix-RTOS
 *
 * Offloading function
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#ifndef COGNIT_APP_OFFLOAD_H
#define COGNIT_APP_OFFLOAD_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

#include <jansson.h>

#include <cognit/device_runtime.h>

#include "devices.h"


#define COGNIT_APP_FUNCTION_MAX_LEN     (16 * 1024)
#define COGNIT_APP_STRING_PARAM_MAX_LEN (1024)

#define OFFLOAD_RESULT_BUSY  (1)
#define OFFLOAD_RESULT_OK    (0)
#define OFFLOAD_RESULT_ERROR (-1)


typedef enum {
	offload_stateUninitialized,
	offload_stateInitializationError,
	offload_stateNone,
	offload_stateInputReady,
	offload_stateInProgress,
	offload_stateResultReady,
	offload_stateError,
} offload_state_t;


typedef struct {
	const char *decisionPath;
	const char *trainingPath;
	const char *evalPath;

	uint32_t trainingTimeoutMs;
	uint32_t decisionTimeoutMs;
	uint32_t evalTimeoutMs;

	scheduling_t decisionReqs;
	scheduling_t trainingReqs;
	cognit_config_t cognit;
} offload_config_t;


typedef enum {
	offload_functionNone,
	offload_functionDecision,
	offload_functionTraining,
	offload_functionEvaluation,
} offload_function_t;


typedef struct {
	devices_info_t *devInfo;
	const char *S3;
	const char *besmart;
	devices_userPref_t *userPref;
	const char *hmm;
	json_t *stateRangeJson;
} offload_decisionInput_t;


typedef struct {
	devices_info_t *devInfo;
	const char *S3;
	const char *besmart;
	devices_userPref_t *userPref;
	const char *hmm;
	const char *trainingParams;
	json_t *stateRangeJson;
} offload_trainingInput_t;


typedef struct {
	device_runtime_t deviceRuntime;
	faas_t faas;
	const offload_config_t *config;

	offload_function_t currentFunction;
	char functionBuffer[COGNIT_APP_FUNCTION_MAX_LEN + 1];

	char paramBuffer[COGNIT_APP_STRING_PARAM_MAX_LEN + 1];

	void *response[3];
	offload_state_t _state;
	pthread_cond_t cond;
	pthread_mutex_t lock;
	pthread_t thread;
} offload_ctx_t;


int offload_decisionAlgo(offload_ctx_t *ctx, const offload_decisionInput_t *in);


int offload_getDecisionResult(offload_ctx_t *ctx, devices_config_t *out);


int offload_training(offload_ctx_t *ctx, const offload_trainingInput_t *in);


int offload_getTrainingnResult(offload_ctx_t *ctx, json_t **stageRange);


int offload_evaluation(offload_ctx_t *ctx, const offload_trainingInput_t *in);


int offload_getEvaluationResult(offload_ctx_t *ctx, bool *result);


int offload_deinitDR(offload_ctx_t *ctx);


int offload_initDR(offload_ctx_t *ctx, const offload_config_t *offloadConfig);

#endif /* COGNIT_APP_OFFLOAD_H */
