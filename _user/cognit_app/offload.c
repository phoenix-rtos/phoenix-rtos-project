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

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <curl/curl.h>

#include <jansson.h>

#include <cognit/device_runtime.h>

#include "devices_json.h"
#include "offload.h"

#define COGNIT_APP_LOG_TAG "cognit_app/offload : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_INFO
#include "logger.h"

#define MAX_STRING_PARAM_LEN (1024)

#ifndef COGNIT_APP_DR_THREAD_STACK_SIZE
#define COGNIT_APP_DR_THREAD_STACK_SIZE (130000)
#endif


static offload_state_t getState(offload_ctx_t *ctx)
{
	offload_state_t state;
	pthread_mutex_lock(&ctx->lock);
	state = ctx->_state;
	pthread_mutex_unlock(&ctx->lock);
	return state;
}


static void setState(offload_ctx_t *ctx, offload_state_t state)
{
	pthread_mutex_lock(&ctx->lock);
	ctx->_state = state;
	pthread_cond_signal(&ctx->cond);
	pthread_mutex_unlock(&ctx->lock);
}


static void waitForReadyInput(offload_ctx_t *ctx)
{
	pthread_mutex_lock(&ctx->lock);
	while (ctx->_state != offload_stateInputReady) {
		pthread_cond_wait(&ctx->cond, &ctx->lock);
	}
	pthread_mutex_unlock(&ctx->lock);
}


#define TRY_ADD_STRING_PARAM(fun__, ...) \
	do { \
		memset(ctx->paramBuffer, '\0', sizeof(ctx->paramBuffer)); \
		int ret__ = fun__(__VA_ARGS__, ctx->paramBuffer, MAX_STRING_PARAM_LEN); \
		if (ret__ < 0) { \
			return OFFLOAD_RESULT_ERROR; \
		} \
		addSTRINGParam(&ctx->faas, ctx->paramBuffer); \
	} while (0)


static int prepareDecisionAlgoInput(offload_ctx_t *ctx, const offload_decisionInput_t *in)
{
	clearFaasArgs(&ctx->faas);

	addUINT64Var(&ctx->faas, in->devInfo->metersim.timestamp);
	addSTRINGParam(&ctx->faas, in->S3);
	addSTRINGParam(&ctx->faas, in->besmart);

	TRY_ADD_STRING_PARAM(devicesJson_homeModelInfoToJson, &in->devInfo->homeModel, NULL);
	TRY_ADD_STRING_PARAM(devicesJson_storageInfoToJson, &in->devInfo->storage[0]);
	TRY_ADD_STRING_PARAM(devicesJson_EVInfoToJson, in->devInfo->ev, in->devInfo->evCount);
	TRY_ADD_STRING_PARAM(devicesJson_heatingInfoToJson, &in->devInfo->heating[0]);
	TRY_ADD_STRING_PARAM(devicesJson_userPrefToJson, in->userPref);

	return OFFLOAD_RESULT_OK;
}


static int prepareTrainingInput(offload_ctx_t *ctx, const offload_trainingInput_t *in)
{
	clearFaasArgs(&ctx->faas);

	addSTRINGParam(&ctx->faas, in->trainingParams);
	addSTRINGParam(&ctx->faas, in->S3);
	addSTRINGParam(&ctx->faas, in->besmart);

	TRY_ADD_STRING_PARAM(devicesJson_homeModelInfoToJson, &in->devInfo->homeModel, NULL);
	TRY_ADD_STRING_PARAM(devicesJson_storageInfoToJson, &in->devInfo->storage[0]);
	TRY_ADD_STRING_PARAM(devicesJson_EVInfoToJson, in->devInfo->ev, in->devInfo->evCount);
	TRY_ADD_STRING_PARAM(devicesJson_heatingInfoToJson, &in->devInfo->heating[0]);
	TRY_ADD_STRING_PARAM(devicesJson_userPrefToJson, in->userPref);

	return OFFLOAD_RESULT_OK;
}


static int parseDecisionAlgoOutput(devices_config_t *out, char **response)
{
	int ret = devicesJson_tempSettingFromJson(&out->heating[0].optimalTemp, response[0], strlen(response[0]));
	if (ret < 0) {
		log_warn("Parsing heating response failed");
		return OFFLOAD_RESULT_ERROR;
	}

	ret = devicesJson_storageParamsFromJson(&out->storage[0], response[1], strlen(response[1]));
	if (ret < 0) {
		log_warn("Parsing storage response failed");
		return OFFLOAD_RESULT_ERROR;
	}

	ret = devicesJson_EVParamsFromJson(&out->ev[0], out->evCount, response[2], strlen(response[2]));
	if (ret < 0) {
		log_warn("Parsing EV response failed");
		return OFFLOAD_RESULT_ERROR;
	}

	return OFFLOAD_RESULT_OK;
}


static int parseEvalResponse(bool *result, char **response)
{
	*result = *((bool *)response[0]);
	return 0;
}


static inline void freeResponse(void **response, size_t cnt)
{
	for (size_t i = 0; i < cnt; i++) {
		free(response[i]);
	}
}


static int loadFunction(offload_ctx_t *ctx, const char *path)
{
	if (path == NULL) {
		return OFFLOAD_RESULT_ERROR;
	}

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		return OFFLOAD_RESULT_ERROR;
	}

	ctx->currentFunction = offload_functionNone;
	memset(ctx->functionBuffer, '\0', sizeof(ctx->functionBuffer));

	int ret;
	size_t bytesRead = 0;
	while (1) {
		ret = read(fd, &ctx->functionBuffer[bytesRead], COGNIT_APP_FUNCTION_MAX_LEN - bytesRead);
		if (ret <= 0) {
			break;
		}

		bytesRead += ret;
	};

	ctx->functionBuffer[bytesRead] = '\0';

	if (ret == 0) {
		log_debug("Read %zu bytes from file %s", bytesRead, path);
		log_debug("functionBuffer len: %zu", strlen(ctx->functionBuffer));
	}
	else {
		log_warn("Reading %s failed with %d", path, errno);
	}

	close(fd);
	return ret;
}


int offload_decisionAlgo(offload_ctx_t *ctx, const offload_decisionInput_t *in)
{
	if (getState(ctx) == offload_stateInputReady) {
		return OFFLOAD_RESULT_ERROR;
	}

	/* Load decision algo function if not cached */
	if (ctx->currentFunction != offload_functionDecision) {
		if (loadFunction(ctx, ctx->config->decisionPath) < 0) {
			return OFFLOAD_RESULT_ERROR;
		}
		ctx->currentFunction = offload_functionDecision;
	}

	/* Add function arguments to FaaS context */
	addFC(&ctx->faas, ctx->functionBuffer);
	ctx->faas.timeout_ms = ctx->config->decisionTimeoutMs;
	if (prepareDecisionAlgoInput(ctx, in) < 0) {
		return -1;
	}

	setState(ctx, offload_stateInputReady);

	return OFFLOAD_RESULT_OK;
}


int offload_getDecisionResult(offload_ctx_t *ctx, devices_config_t *out)
{
	offload_state_t state = getState(ctx);
	if (state == offload_stateInputReady) {
		return OFFLOAD_RESULT_BUSY;
	}

	if (state != offload_stateResultReady) {
		setState(ctx, offload_stateNone);
		return OFFLOAD_RESULT_ERROR;
	}

	/* Parse response */
	if (parseDecisionAlgoOutput(out, (char **)ctx->response) < 0) {
		freeResponse(ctx->response, 3);
		setState(ctx, offload_stateNone);
		return OFFLOAD_RESULT_ERROR;
	}

	freeResponse(ctx->response, 3);
	setState(ctx, offload_stateNone);
	return OFFLOAD_RESULT_OK;
}


int offload_training(offload_ctx_t *ctx, const offload_trainingInput_t *in)
{
	if (getState(ctx) == offload_stateInputReady) {
		return OFFLOAD_RESULT_ERROR;
	}

	/* Load training algo function if not cached */
	if (ctx->currentFunction != offload_functionTraining) {
		if (loadFunction(ctx, ctx->config->trainingPath) < 0) {
			return OFFLOAD_RESULT_ERROR;
		}
		ctx->currentFunction = offload_functionTraining;
	}

	/* Add function arguments to FaaS context */
	addFC(&ctx->faas, ctx->functionBuffer);
	ctx->faas.timeout_ms = ctx->config->trainingTimeoutMs;
	if (prepareTrainingInput(ctx, in) < 0) {
		return -1;
	}

	setState(ctx, offload_stateInputReady);
	return OFFLOAD_RESULT_OK;
}


int offload_getTrainingnResult(offload_ctx_t *ctx, json_t **stageRange)
{
	offload_state_t state = getState(ctx);
	if (state == offload_stateInputReady) {
		return OFFLOAD_RESULT_BUSY;
	}

	if (state != offload_stateResultReady) {
		setState(ctx, offload_stateNone);
		return OFFLOAD_RESULT_ERROR;
	}

	freeResponse(ctx->response, 1);
	setState(ctx, offload_stateNone);
	return OFFLOAD_RESULT_OK;
}


int offload_evaluation(offload_ctx_t *ctx, const offload_trainingInput_t *in)
{
	if (getState(ctx) == offload_stateInputReady) {
		return OFFLOAD_RESULT_ERROR;
	}

	/* Load eval function if not cached */
	if (ctx->currentFunction != offload_functionEvaluation) {
		if (loadFunction(ctx, ctx->config->evalPath) < 0) {
			return OFFLOAD_RESULT_ERROR;
		}
		ctx->currentFunction = offload_functionEvaluation;
	}

	/* Add function arguments to FaaS context */
	addFC(&ctx->faas, ctx->functionBuffer);
	ctx->faas.timeout_ms = ctx->config->evalTimeoutMs;
	if (prepareTrainingInput(ctx, in) < 0) {
		return -1;
	}

	setState(ctx, offload_stateInputReady);
	return OFFLOAD_RESULT_OK;
}


int offload_getEvaluationResult(offload_ctx_t *ctx, bool *result)
{
	offload_state_t state = getState(ctx);
	if (state == offload_stateInputReady) {
		return OFFLOAD_RESULT_BUSY;
	}

	if (state != offload_stateResultReady) {
		setState(ctx, offload_stateNone);
		return OFFLOAD_RESULT_ERROR;
	}

	/* Parse response */
	if (parseEvalResponse(result, (char **)ctx->response) < 0) {
		freeResponse(ctx->response, 1);
		setState(ctx, offload_stateNone);
		return OFFLOAD_RESULT_ERROR;
	}

	freeResponse(ctx->response, 1);
	setState(ctx, offload_stateNone);
	return OFFLOAD_RESULT_OK;
}


/* Enables async offloading */
static void *deviceRuntimeThread(void *args)
{
	offload_ctx_t *ctx = (offload_ctx_t *)args;

	log_debug("Device runtime TID: %d", gettid());

	if (device_runtime_init(&ctx->deviceRuntime, ctx->config->cognit, ctx->config->decisionReqs, &ctx->faas) < 0) {
		log_error("device_runtime_init failed");
		setState(ctx, offload_stateInitializationError);
		return NULL;
	}

	setState(ctx, offload_stateNone);

	while (1) {
		waitForReadyInput(ctx);

		for (size_t i = 0; i < ctx->faas.params_count; i++) {
			log_debug("Param no %zu: %s", i, ctx->faas.params[i].param.my_string);
		}

		const scheduling_t *reqs = ctx->currentFunction == offload_functionTraining ? &ctx->config->trainingReqs : &ctx->config->decisionReqs;
		e_status_code_t status = device_runtime_call(&ctx->deviceRuntime, &ctx->faas, *reqs, ctx->response);
		setState(ctx, status == E_ST_CODE_SUCCESS ? offload_stateResultReady : offload_stateError);
	}

	/* This function never returns */
	return NULL;
}


int offload_deinitDR(offload_ctx_t *ctx)
{
	/* TODO: pthread */
	return OFFLOAD_RESULT_OK;
}


int offload_initDR(offload_ctx_t *ctx, const offload_config_t *offloadConfig)
{
	ctx->config = offloadConfig;
	ctx->currentFunction = offload_functionNone;

	ctx->_state = offload_stateUninitialized;

	/* TODO: error handling */
	pthread_mutex_init(&ctx->lock, NULL);

	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_cond_init(&ctx->cond, &condattr);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, COGNIT_APP_DR_THREAD_STACK_SIZE);
	pthread_create(&ctx->thread, &attr, deviceRuntimeThread, ctx);

	pthread_mutex_lock(&ctx->lock);
	while (ctx->_state == offload_stateUninitialized) {
		pthread_cond_wait(&ctx->cond, &ctx->lock);
	}
	pthread_mutex_unlock(&ctx->lock);

	if (getState(ctx) == offload_stateInitializationError) {
		log_warn("Initializing COGNIT Device Runtime failed");
		return OFFLOAD_RESULT_ERROR;
	}

	log_info("Initialized COGNIT Device Runtime");
	return OFFLOAD_RESULT_OK;
}
