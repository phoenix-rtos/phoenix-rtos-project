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

#ifndef COGNIT_APP_DEVICES_JSON_H
#define COGNIT_APP_DEVICES_JSON_H

#include "devices.h"


int devicesJson_storageInfoToJson(const devices_storageInfo_t *storage, char *out, size_t outLen);


int devicesJson_EVInfoToJson(const devices_EVInfo_t *ev, char *buf, size_t bufSize);


int devicesJson_metersimInfoToJson(const devices_metersimInfo_t *metersim, char *buf, size_t bufSize);


int devicesJson_heatingInfoToJson(const devices_heatingInfo_t *heating, char *buf, size_t bufSize);


int devicesJson_homeModelInfoToJson(const devices_homeModelInfo_t *home, char *buf, size_t bufsize);


int devicesJson_userPrefToJson(const devices_userPref_t *pref, char *buf, size_t bufsize);


int devicesJson_tempSettingFromJson(float *val, const char *buf, size_t buflen);


int devicesJson_storageParamsFromJson(devices_storageParams_t *storage, const char *buf, size_t buflen);


int devicesJson_EVParamsFromJson(devices_EVParams_t *ev, const char *buf, size_t buflen);


int devicesJson_heatingParamsFromJson(devices_heatingParams_t *heating, const char *buf, size_t buflen);


#endif /* COGNIT_APP_DEVICES_JSON_H */
