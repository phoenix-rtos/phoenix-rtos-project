/*
 * Phoenix-RTOS
 *
 * Devices json ser/deser
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#include <stddef.h>
#include <stdbool.h>

#include <jansson.h>
#include "devices_json.h"


#define TRY_ADD(root__, type__, key__, ...) \
	({ \
		json_t *obj__ = json_##type__(__VA_ARGS__); \
		if (obj__ == NULL) { \
			json_decref(root__); \
			return -1; \
		} \
		if (json_object_set_new(root__, key__, obj__) < 0) { \
			json_decref(root__); \
			return -1; \
		} \
		obj__; \
	})

#define TRY_GET(root__, type__, key__) \
	({ \
		json_t *obj__ = json_object_get(root__, key__); \
		if (obj__ == NULL) { \
			json_decref(root__); \
			return -1; \
		} \
		json_##type__##_value(obj__); \
	})


int devicesJson_storageInfoToJson(const devices_storageInfo_t *storage, char *buf, size_t bufSize)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD(root, real, "max_capacity", storage->maxCapacity);
	TRY_ADD(root, real, "min_charge_level", storage->minChargeLevel);
	TRY_ADD(root, real, "charging_switch_level", storage->chargingSwitchLevel);
	TRY_ADD(root, real, "curr_charge_level", storage->currChangeLevel);
	TRY_ADD(root, real, "nominal_power", storage->nominalPower);
	TRY_ADD(root, real, "efficiency", storage->efficiency);
	TRY_ADD(root, real, "energy_loss", storage->energyLoss);

	size_t ret = json_dumpb(root, buf, bufSize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(root);
	return 0;
}


int devicesJson_EVInfoToJson(const devices_EVInfo_t *ev, char *buf, size_t bufSize)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD(root, real, "max_capacity", ev->maxCapacity);
	TRY_ADD(root, real, "min_charge_level", ev->minChargeLevel);
	TRY_ADD(root, real, "charging_switch_level", ev->chargingSwitchLevel);
	TRY_ADD(root, real, "curr_charge_level", ev->currChangeLevel);
	TRY_ADD(root, real, "nominal_power", ev->nominalPower);
	TRY_ADD(root, real, "efficiency", ev->efficiency);
	TRY_ADD(root, real, "energy_loss", ev->energyLoss);
	TRY_ADD(root, real, "driving_charge_level", ev->drivingPower);
	TRY_ADD(root, integer, "time_until_charged", 7200);
	TRY_ADD(root, boolean, "is_available", ev->isAvailable);

	json_t *xd = json_object();
	json_object_set_new(xd, "0", root);

	size_t ret = json_dumpb(xd, buf, bufSize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(xd);
	return 0;
}


int devicesJson_metersimInfoToJson(const devices_metersimInfo_t *metersim, char *buf, size_t bufSize)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD(root, integer, "aplus_total", (int64_t)metersim->aplus);
	TRY_ADD(root, integer, "aminus_total", (int64_t)metersim->aminus);

	size_t ret = json_dumpb(root, buf, bufSize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(root);
	return 0;
}


int devicesJson_heatingInfoToJson(const devices_heatingInfo_t *heating, char *buf, size_t bufSize)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD(root, real, "curr_temp", heating->currTemp);
	TRY_ADD(root, real, "preferred_temp", heating->optimalTemp);

	json_t *arr = TRY_ADD(root, array, "powers_of_heating_devices");
	for (int i = 0; i < heating->heatDevCnt; i++) {
		json_t *val = json_real(heating->heatDevPwr[i]);
		if (val == NULL || json_array_append_new(arr, val) < 0) {
			json_decref(root);
			return -1;
		}
	}

	size_t ret = json_dumpb(root, buf, bufSize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(root);
	return 0;
}


int devicesJson_homeModelInfoToJson(const devices_homeModelInfo_t *home, char *buf, size_t bufsize)
{
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	TRY_ADD(root, real, "min_temp_setting", home->minTempSetting);
	TRY_ADD(root, real, "max_temp_setting", home->maxTempSetting);
	TRY_ADD(root, real, "heating_delta_temperature", home->heatingDeltaTemperature);
	TRY_ADD(root, real, "heating_coefficient", home->heatingCoeff);
	TRY_ADD(root, real, "heat_loss_coefficient", home->heatLossCoeff);
	TRY_ADD(root, real, "heat_capacity", home->heatCapacity);

	size_t ret = json_dumpb(root, buf, bufsize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(root);

	return 0;
}

int devicesJson_userPrefToJson(const devices_userPref_t *pref, char *buf, size_t bufsize)
{
	json_error_t err;
	json_t *root = json_object();
	if (root == NULL) {
		return -1;
	}

	json_t *ev = json_loads(pref->ev, 0, &err);
	json_t *temp = json_loads(pref->temp, 0, &err);

	json_object_set_new(root, "ev_driving_schedule", ev);
	json_object_set_new(root, "pref_temp_schedule", temp);
	TRY_ADD(root, integer, "cycle_timedelta_s", pref->offloadFrequency);

	size_t ret = json_dumpb(root, buf, bufsize, JSON_REAL_PRECISION(5));
	buf[ret] = '\0';

	json_decref(root);

	return 0;
}


int devicesJson_tempSettingFromJson(float *val, const char *buf, size_t buflen)
{
	json_error_t err;
	json_t *root = json_loads(buf, 0, &err);
	if (root == NULL) {
		return -1;
	}

	*val = TRY_GET(root, real, "temp");

	json_decref(root);
	return 0;
}


int devicesJson_storageParamsFromJson(devices_storageParams_t *storage, const char *buf, size_t buflen)
{
	json_error_t err;
	json_t *root = json_loads(buf, 0, &err);
	if (root == NULL) {
		return -1;
	}

	storage->inWRte = TRY_GET(root, real, "InWRte");
	storage->outWRte = TRY_GET(root, real, "OutWRte");
	storage->storCtl = TRY_GET(root, integer, "StorCtl_Mod");

	json_decref(root);
	return 0;
}


int devicesJson_EVParamsFromJson(devices_EVParams_t *ev, const char *buf, size_t buflen)
{
	json_error_t err;
	json_t *root = json_loads(buf, 0, &err);
	if (root == NULL) {
		return -1;
	}

	json_t *first = json_object_get(root, "0");
	if (first == NULL) {
		json_decref(root);
		return -1;
	}

	ev->inWRte = TRY_GET(first, real, "InWRte");
	ev->outWRte = TRY_GET(first, real, "OutWRte");
	ev->storCtl = TRY_GET(first, integer, "StorCtl_Mod");

	json_decref(root);
	return 0;
}


int devicesJson_heatingParamsFromJson(devices_heatingParams_t *heating, const char *buf, size_t buflen)
{
	return 0;
}
