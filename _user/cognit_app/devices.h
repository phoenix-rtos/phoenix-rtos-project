/*
 * Phoenix-RTOS
 *
 * Devices handlers for Cognit demo
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */


#ifndef COGNIT_DEVICES_H
#define COGNIT_DEVICES_H

#include <stdbool.h>
#include <stdint.h>

#include <modbus.h>


#define COGNIT_DEVICES_HEAT_NAME_MAX_LEN (16)
#define COGNIT_DEVICES_HEAT_DEV_MAX_CNT  (8)
#define COGNIT_DEVICES_DEV_MAX_COUNT     (4)
#define COGNIT_DEVICES_MAX_PREF_LEN      (16)

#define COGNIT_DEVICES_MODBUS_REGS_MAX_COUNT (123)


typedef struct {
	float maxCapacity;
	float minChargeLevel;
	float chargingSwitchLevel;
	float currChangeLevel;
	float nominalPower;
	float efficiency;
	float energyLoss;
} devices_storageInfo_t;


typedef struct {
	float maxCapacity;
	float minChargeLevel;
	float chargingSwitchLevel;
	float currChangeLevel;
	float nominalPower;
	float efficiency;
	float energyLoss;
	float drivingPower;
	bool isAvailable;
} devices_EVInfo_t;


typedef struct {
	uint64_t aplus;
	uint64_t aminus;
	uint64_t timestamp; /* Current time of the simulation */
} devices_metersimInfo_t;


typedef struct {
	char name[COGNIT_DEVICES_HEAT_NAME_MAX_LEN + 1];

	float currTemp;
	float optimalTemp;

	uint8_t heatDevCnt;
	float heatDevPwr[COGNIT_DEVICES_HEAT_DEV_MAX_CNT];
	bool heatDevOn[COGNIT_DEVICES_HEAT_DEV_MAX_CNT];
} devices_heatingInfo_t;


typedef struct {
	float minTempSetting;
	float maxTempSetting;
	float heatingDeltaTemperature;
	float heatingCoeff;
	float heatLossCoeff;
	float heatCapacity;
} devices_homeModelInfo_t;


typedef struct {
	bool initialized;
	char ev[256];               /* JSON */
	char temp[256];             /* JSON */
	uint32_t offloadFrequency;  /* sec */
	uint32_t trainingFrequency; /* sec */
	bool offloadPredictNow;
	bool offloadTrainingNow;
} devices_userPref_t;


typedef struct {
	float inWRte;
	float outWRte;
	uint16_t storCtl;
} devices_storageParams_t;


typedef struct {
	float inWRte;
	float outWRte;
	uint16_t storCtl;
} devices_EVParams_t;


typedef struct {
	float optimalTemp;
} devices_heatingParams_t;


/* In real-life setting, this would be a configuration file of the application */
typedef struct {
	uint8_t storageCount;
	uint8_t evCount;
	uint8_t roomCount;
	uint8_t pvCount;
	uint16_t speedup;
} devices_simulationData_t;


typedef struct {
	uint64_t timestamp;
	uint64_t cycle;

	devices_metersimInfo_t metersim;
	devices_homeModelInfo_t homeModel;

	devices_storageInfo_t storage[COGNIT_DEVICES_DEV_MAX_COUNT];
	devices_EVInfo_t ev[COGNIT_DEVICES_DEV_MAX_COUNT];
	devices_heatingInfo_t heating[COGNIT_DEVICES_DEV_MAX_COUNT];
} devices_info_t;

typedef struct {
	devices_storageParams_t storage[COGNIT_DEVICES_DEV_MAX_COUNT];
	devices_EVParams_t ev[COGNIT_DEVICES_DEV_MAX_COUNT];
	devices_heatingParams_t heating[COGNIT_DEVICES_DEV_MAX_COUNT];
} devices_config_t;


typedef struct {
	devices_simulationData_t simData;

	uint16_t regsBuffer[COGNIT_DEVICES_MODBUS_REGS_MAX_COUNT];
	modbus_t *modbus;
} devices_ctx_t;


int devices_getInfo(devices_ctx_t *ctx, devices_info_t *info);


int devices_setParams(devices_ctx_t *ctx, const devices_config_t *config);


int devices_getMetersimInfo(devices_ctx_t *ctx, devices_metersimInfo_t *metersim);


int devices_getTimeMetersim(devices_ctx_t *ctx, uint64_t *val);


int devices_getUserPref(devices_ctx_t *ctx, devices_userPref_t *pref);


/*
 * Informs the simulation that training is taking place, so that the simulation
 * can decrease the speed of time passing, to wait for training results.
 * */
int devices_simulationSetTrainingState(devices_ctx_t *ctx, bool state);


/* Devices initializer assumes the modbus library to be already initialized */
int devices_init(devices_ctx_t *ctx, modbus_t *modbus);

#endif /* COGNIT_DEVICES_H */
