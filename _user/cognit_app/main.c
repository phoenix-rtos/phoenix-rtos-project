/*
 * Phoenix-RTOS
 *
 * Cognit demo application
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <jansson.h>
#include <curl/curl.h>

#include "devices.h"
#include "offload.h"
#include "devices_json.h"
#include "modbus_comm.h"
#include "besmart.h"

#define COGNIT_APP_LOG_TAG "cognit_app : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_DEBUG
#include "logger.h"


#define MAIN_LOOP_SLEEP_US            (500 * 1000)
#define MODBUS_READ_WRITE_TIMEOUT_MS  (3000)
#define USER_PREF_UPDATE_FREQ_SEC_RTC (5)

#define SECONDS_IN_DAY (24 * 60 * 60)

static struct {
	offload_ctx_t offload;
	offload_config_t offloadConfig;

	besmart_ctx_t besmart;

	uint16_t historyTimeDeltaDays; /* for besmart */
	json_t *besmartOffloadParams;
	json_t *stateRange;

	modbus_t modbus;
	devices_ctx_t devices;
	devices_info_t devInfo;
	devices_config_t devConfig;
	devices_userPref_t userPref;

	uint16_t userPrefUpdFreqSec;

	struct {
		char url[256];
		char user[128];
		char pwd[128];
		char decisionPath[PATH_MAX + 1];
		char trainPath[PATH_MAX + 1];
	} cognitConfig;

	struct {
		besmart_sensor_t sensorAplus;
		besmart_sensor_t sensorAminus;
		char login[64];
		char pwd[64];
	} besmartConfig;

	struct {
		char S3[1024];
		char besmart[1024];
		char userPref[1024];
		char training[2048];
	} offloadFunParams;
} common;


#define TRY_RAISE(expr__) \
	do { \
		int ret__ = expr__; \
		if (ret__ < 0) { \
			return -1; \
		} \
	} while (0)


#if 0
static uint64_t getTimeMonoMs(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / (1000 * 1000);
}
#endif


static uint64_t getTimeMetersim(void)
{
	static uint64_t timeMetrsim = 0;
	uint64_t val;
	if (devices_getTimeMetersim(&common.devices, &val) < 0) {
		log_warn("devices_getTimeMetersim failed");
	}
	else {
		timeMetrsim = val;
	}
	return timeMetrsim;
}


static int sendBesmartData(void)
{
	devices_metersimInfo_t metersim;

	if (devices_getMetersimInfo(&common.devices, &metersim) < 0) {
		log_warn("devices_getMetersimInfo failed");
		return -1;
	}

	besmart_signalData_t dataAPlus = {
		.origin = 1,
		.timestamp = metersim.timestamp * 1000,
		.type = besmart_typeFloat,
		.value.fl = (double)metersim.aplus / 1000.0,
	};

	besmart_signalData_t dataAMinus = {
		.origin = 1,
		.timestamp = metersim.timestamp * 1000,
		.type = besmart_typeFloat,
		.value.fl = (float)metersim.aminus / 1000.0,
	};

	if (besmart_updateSignal(&common.besmart, &common.besmartConfig.sensorAplus, &dataAPlus) < 0) {
		log_warn("Updating A+ signal failed");
		return -1;
	}

	if (besmart_updateSignal(&common.besmart, &common.besmartConfig.sensorAminus, &dataAMinus) < 0) {
		log_warn("Updating A- signal failed");
		return -1;
	}

	log_info("Sent energy data to besmart.energy");
	return 0;
}


static int updateUserPref(void)
{
	if (devices_getUserPref(&common.devices, &common.userPref) < 0) {
		log_warn("devices_getUserPref failed");
		return -1;
	}

	return 0;
}


static int runDecision(void)
{
	if (devices_getInfo(&common.devices, &common.devInfo) < 0) {
		log_warn("devices_getInfo failed");
		return -1;
	}

	offload_decisionInput_t in = {
		.devInfo = &common.devInfo,
		.S3 = common.offloadFunParams.S3,
		.besmart = common.offloadFunParams.besmart,
		.userPref = &common.userPref,
		.stateRangeJson = common.stateRange,
	};

	if (offload_decisionAlgo(&common.offload, &in) < 0) {
		log_warn("offload_decisionAlgo failed");
		return -1;
	}

	log_info("Decision algorithm offloaded");
	return 0;
}


static int dumpBesmartOffloadParams(uint64_t now)
{
	json_t *obj = common.besmartOffloadParams;

	json_t *since = json_object_get(obj, "since");
	if (since == NULL) {
		since = json_integer(0);
		if (since == NULL) {
			return -1;
		}
	}
	json_integer_set(since, (int64_t)now - (int64_t)common.userPref.offloadFrequency - (int64_t)common.historyTimeDeltaDays * SECONDS_IN_DAY);

	json_t *till = json_object_get(obj, "till");
	if (till == NULL) {
		till = json_integer(0);
		if (till == NULL) {
			return -1;
		}
	}
	json_integer_set(till, (int64_t)(now - common.userPref.offloadFrequency));

	size_t len = json_dumpb(obj, common.offloadFunParams.besmart, sizeof(common.offloadFunParams.besmart) - 1, JSON_REAL_PRECISION(4)); /* TODO: are we handling errors properly here? */
	common.offloadFunParams.besmart[len] = '\0';

	return 0;
}


static int runTraining(uint64_t now)
{
	if (devices_getInfo(&common.devices, &common.devInfo) < 0) {
		log_warn("devices_getInfo failed");
		return -1;
	}

	if (dumpBesmartOffloadParams(now) < 0) {
		log_warn("dumpBesmartOffloadParams failed");
		return -1;
	}

	offload_trainingInput_t in = {
		.devInfo = &common.devInfo,
		.S3 = common.offloadFunParams.S3,
		.besmart = common.offloadFunParams.besmart,
		.userPref = &common.userPref,
		.trainingParams = common.offloadFunParams.training,
		.stateRangeJson = common.stateRange,
	};

	if (offload_training(&common.offload, &in) < 0) {
		log_warn("offload_training failed");
		return -1;
	}

	devices_simulationSetTrainingState(&common.devices, true);

	log_info("Training algorithm offloaded");
	return 0;
}


static int executeDecisionResult(void)
{
	devices_config_t *config = &common.devConfig;
	config->evCount = common.devConfig.evCount; /* TODO: This is an ugly hack. Make it prettier. */

	int ret = offload_getDecisionResult(&common.offload, config);
	if (ret > 0) {
		return 1; /* Still waiting for the result */
	}
	else if (ret < 0) {
		log_warn("Obtaining decision algorithm results failed");
		return -1;
	}

	if (devices_setParams(&common.devices, config) < 0) {
		log_warn("devices_setParams failed");
		return -1;
	}

	log_success("Applied decision algorithm results");
	return 0;
}


static int getTrainingResult(void)
{
	json_t *tmp;
	int ret = offload_getTrainingnResult(&common.offload, &tmp);
	if (ret > 0) {
		return 1; /* Still waiting for the result */
	}

	if (ret < 0) {
		log_warn("Obtaining training result failed");
	}
	else {
		if (common.stateRange != NULL) {
			json_decref(common.stateRange);
		}
		common.stateRange = tmp;
		log_success("Training finished successfully");
	}

	devices_simulationSetTrainingState(&common.devices, false);
	return 0;
}


static void mainLoop(void)
{
	uint64_t now = getTimeMetersim();
	uint64_t lastDecision = 0;
	uint64_t lastBesmart = 0;
	uint64_t lastUserPref = 0;
	uint64_t lastTraining = 0; /* Wait till we have enough data to train */

	bool decisionOffloaded = false;
	bool trainingOffloaded = false;

	while (1) { /* TODO: maybe state machine? */
		usleep(MAIN_LOOP_SLEEP_US);
		now = getTimeMetersim();

		if (decisionOffloaded) {
			if (executeDecisionResult() == 1) {
				continue; /* Still waiting for results */
			}
			decisionOffloaded = false;
		}

		if (trainingOffloaded) {
			if (getTrainingResult() == 1) {
				continue; /* Still waiting for results */
			}
			trainingOffloaded = false;
		}

		if (lastUserPref == 0 || now - lastUserPref > common.userPrefUpdFreqSec * common.devices.simData.speedup) {
			updateUserPref();
			lastUserPref = now;
		}

		if (common.userPref.offloadTrainingNow || now - lastTraining > common.userPref.trainingFrequency) {
			runTraining(now);
			lastTraining = now;
			trainingOffloaded = true;
			common.userPref.offloadTrainingNow = false;
			continue;
		}

		if (common.userPref.offloadPredictNow || now - lastDecision > common.userPref.offloadFrequency) {
			runDecision();
			lastDecision = now;
			decisionOffloaded = true;
			common.userPref.offloadPredictNow = false;
			continue;
		}

		if (now - lastBesmart > 900) {
			sendBesmartData();
			lastBesmart = now;
			continue;
		}
	}
}


static int jsonGetString(json_t *root, const char *key, char *buf, size_t buflen)
{
	json_t *obj = json_object_get(root, key);
	if (obj == NULL) {
		return -1;
	}

	const char *str = json_string_value(obj);
	if (str == NULL) {
		return -1;
	}

	size_t len = json_string_length(obj);
	if (len > buflen) {
		return -1;
	}

	strncpy(buf, str, buflen);
	return 0;
}


static int readScheduling(json_t *json, scheduling_t *reqs)
{
	TRY_RAISE(jsonGetString(json, "flavour", reqs->flavour, sizeof(reqs->flavour)));

	// TRY_RAISE(jsonGetString(json, "geolocation", reqs->geolocation, sizeof(reqs->geolocation)));

	json_t *obj = json_object_get(json, "max_latency");
	if (obj == NULL) {
		return -1;
	}
	reqs->max_latency = json_integer_value(obj);

	obj = json_object_get(json, "min_renewable");
	if (obj == NULL) {
		return -1;
	}
	reqs->min_renewable = json_integer_value(obj);

	obj = json_object_get(json, "max_fnexec_time");
	if (obj == NULL) {
		return -1;
	}
	reqs->max_function_execution_time = json_real_value(obj);

	json_t *geolocation = json_object_get(json, "geolocation");
	if (geolocation == NULL) {
		return -1;
	}

	obj = json_object_get(geolocation, "latitude");
	if (obj == NULL) {
		return -1;
	}
	reqs->geolocation.latitude = json_real_value(obj);

	obj = json_object_get(geolocation, "longitude");
	if (obj == NULL) {
		return -1;
	}
	reqs->geolocation.longitude = json_real_value(obj);

	return 0;
}


static int readCognitConfig(json_t *cognit)
{
	TRY_RAISE(jsonGetString(cognit, "url", common.cognitConfig.url, sizeof(common.cognitConfig.url)));
	TRY_RAISE(jsonGetString(cognit, "user", common.cognitConfig.user, sizeof(common.cognitConfig.user)));
	TRY_RAISE(jsonGetString(cognit, "pwd", common.cognitConfig.pwd, sizeof(common.cognitConfig.pwd)));

	json_t *training = json_object_get(cognit, "training");
	if (training == NULL) {
		return -1;
	}
	if (readScheduling(training, &common.offloadConfig.trainingReqs) < 0) {
		return -1;
	}

	json_t *decision = json_object_get(cognit, "decision");
	if (decision == NULL) {
		return -1;
	}
	if (readScheduling(decision, &common.offloadConfig.decisionReqs) < 0) {
		return -1;
	}

	return 0;
}


static int readBesmartConfig(json_t *besmart)
{
	TRY_RAISE(jsonGetString(besmart, "login", common.besmartConfig.login, sizeof(common.besmartConfig.login)));
	TRY_RAISE(jsonGetString(besmart, "pwd", common.besmartConfig.pwd, sizeof(common.besmartConfig.pwd)));

	json_t *obj = json_object_get(besmart, "cid");
	if (obj == NULL) {
		return -1;
	}
	int cid = json_integer_value(obj);

	obj = json_object_get(besmart, "mid");
	if (obj == NULL) {
		return -1;
	}
	int mid = json_integer_value(obj);

	common.besmartConfig.sensorAplus = (besmart_sensor_t) {
		.cid = cid,
		.mid = mid,
		.moid = 32, /* A+ */
	};

	common.besmartConfig.sensorAminus = (besmart_sensor_t) {
		.cid = cid,
		.mid = mid,
		.moid = 34, /* A- */
	};

	return 0;
}


static int readAppConfig(json_t *app)
{
	TRY_RAISE(jsonGetString(app, "decisionPath", common.cognitConfig.decisionPath, sizeof(common.cognitConfig.decisionPath)));
	TRY_RAISE(jsonGetString(app, "trainPath", common.cognitConfig.trainPath, sizeof(common.cognitConfig.trainPath)));

	json_t *obj = json_object_get(app, "trainTimeOut");
	if (obj == NULL) {
		return -1;
	}
	common.offloadConfig.trainingTimeoutMs = json_integer_value(obj);

	obj = json_object_get(app, "decisionTimeOut");
	if (obj == NULL) {
		return -1;
	}
	common.offloadConfig.decisionTimeoutMs = json_integer_value(obj);

	obj = json_object_get(app, "userPrefUpdFreqSec");
	if (obj == NULL) {
		return -1;
	}
	common.userPrefUpdFreqSec = json_integer_value(obj);
	return 0;
}


static int readConfigFile(const char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		log_error("Could not open config file %s", path);
		return -1;
	}

	json_error_t err;
	json_t *config = json_loadfd(fd, 0, &err);
	if (config == NULL) {
		log_error("Parsing config json failed");
		close(fd);
		return -1;
	}

	json_t *cognit = json_object_get(config, "cognit");
	if (cognit == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}

	if (readCognitConfig(cognit) < 0) {
		json_decref(config);
		close(fd);
		return -1;
	}

	json_t *besmartSend = json_object_get(config, "besmartSend");
	if (besmartSend == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}

	if (readBesmartConfig(besmartSend) < 0) {
		json_decref(config);
		close(fd);
		return -1;
	}

	common.besmartOffloadParams = json_object_get(config, "besmartOffload");
	if (common.besmartOffloadParams == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}
	size_t len = json_dumpb(common.besmartOffloadParams, common.offloadFunParams.besmart, sizeof(common.offloadFunParams.besmart) - 1, JSON_REAL_PRECISION(4));
	common.offloadFunParams.besmart[len] = '\0';
	json_incref(common.besmartOffloadParams); /* Will be modified and used later */

	json_t *S3 = json_object_get(config, "S3");
	if (S3 == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}
	len = json_dumpb(S3, common.offloadFunParams.S3, sizeof(common.offloadFunParams.S3) - 1, JSON_REAL_PRECISION(4));
	common.offloadFunParams.S3[len] = '\0';

	json_t *training = json_object_get(config, "trainParams");
	if (training == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}

	json_t *timeDeltaDays = json_object_get(training, "history_timedelta_days");
	if (timeDeltaDays == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}
	common.historyTimeDeltaDays = json_integer_value(timeDeltaDays);

	len = json_dumpb(training, common.offloadFunParams.training, sizeof(common.offloadFunParams.training) - 1, JSON_REAL_PRECISION(4));
	common.offloadFunParams.training[len] = '\0';

	json_t *app = json_object_get(config, "app");
	if (app == NULL) {
		json_decref(config);
		close(fd);
		return -1;
	}

	if (readAppConfig(app) < 0) {
		json_decref(config);
		close(fd);
		return -1;
	}

	json_decref(config);
	close(fd);
	return 0;
}


static void printUsage(const char *name)
{
	/* TODO: */
}


int main(int argc, char **argv)
{
	int opt;
	int baudrate = 9600; /* TODO:  */

	const char *configFile = NULL;

	/* Modbus */
	const char *serialDev = NULL;
	const char *serverIpAddr = NULL;
	const char *serverIpPort = NULL;

	while ((opt = getopt(argc, argv, "c:p:s:a:")) != -1) {
		switch (opt) {
			case 'c':
				configFile = optarg;
				break;

			case 'a':
				serverIpAddr = optarg;
				break;

			case 'p':
				serverIpPort = optarg;
				break;

			case 's':
				serialDev = optarg;
				break;

			default:
				printUsage(argv[0]);
				return EXIT_FAILURE;
		}
	}

	if (configFile == NULL) {
		log_error("Pass config file");
		printUsage(argv[0]);
		return (EXIT_FAILURE);
	}


	/* Read config file */
	if (readConfigFile(configFile) < 0) {
		log_error("Reading config file failed");
		return EXIT_FAILURE;
	}


	/* Initialize communication with simulation */
	if (serialDev) {
		if (modbusComm_initSerial(serialDev, baudrate) < 0) {
			log_error("modbusComm_initSerial failed");
			return EXIT_FAILURE;
		}
	}
	else if (serverIpAddr && serverIpPort) {
		if (modbusComm_initTcp(serverIpAddr, serverIpPort) < 0) {
			log_error("modbusComm_initTcp failed");
			return EXIT_FAILURE;
		}
	}
	else {
		log_error("Pass serial dev or server ip address");
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}


	/* Initialize libmodbus */
	modbus_callbacks_t modbusCb = {
		.read = modbusComm_read,
		.write = modbusComm_write,
		.userArgs = NULL,
	};
	modbus_init(&common.modbus, &modbusCb);
	modbus_setTimeouts(&common.modbus, MODBUS_READ_WRITE_TIMEOUT_MS, MODBUS_READ_WRITE_TIMEOUT_MS); /* TODO: make these timeouts work properly */


	/* Initialize devices module */
	if (devices_init(&common.devices, &common.modbus) < 0) {
		log_error("Could not initialize devices");
		return EXIT_FAILURE;
	}


	/* Initialize libcurl */
	if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
		log_error("curl_global_init failed");
		return EXIT_FAILURE;
	}


	/* Initialize COGNIT Device Runtime */
	common.offloadConfig.decisionPath = common.cognitConfig.decisionPath;
	common.offloadConfig.trainingPath = common.cognitConfig.trainPath;
	common.offloadConfig.cognit = (cognit_config_t) {
		.cognit_frontend_endpoint = common.cognitConfig.url,
		.cognit_frontend_usr = common.cognitConfig.user,
		.cognit_frontend_pwd = common.cognitConfig.pwd,
	};

	if (offload_initDR(&common.offload, &common.offloadConfig) < 0) {
		log_error("Could not initialize device runtime");
		return EXIT_FAILURE;
	}


	/* Initialize besmart.energy client */
	besmart_init(&common.besmart);
	besmart_setUserData(&common.besmart, common.besmartConfig.login, common.besmartConfig.pwd);


	/* Start the main loop */
	mainLoop();
}
