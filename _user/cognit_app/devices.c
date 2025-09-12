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

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <termios.h>

#include <modbus.h>

#include "devices.h"


#define COGNIT_APP_LOG_TAG "cognit_app/devices : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_INFO
#include "logger.h"


#define ADDRESS_SIMULATION_CONTROL (0x01)
#define ADDRESS_METERSIM           (0x02)
#define ADDRESS_EV_PREF            (0x03)
#define ADDRESS_HEATING_PREF       (0x04)
#define ADDRESS_HOME_MODEL         (0x05)
#define ADDRESS_FIRST_PV           (0x10)
#define ADDRESS_FIRST_EV           (0x20)
#define ADDRESS_FIRST_STORAGE      (0x30)
#define ADDRESS_FIRST_HEATING      (0x40)

#define REGISTER_INFO   (0x01)
#define REGISTER_CONFIG (0x41)


static inline float floatFromRegs(const uint16_t *regs)
{
	uint32_t u32 = (regs[0] << 16) + regs[1];
	return *(float *)(&u32);
}


static inline void floatToRegs(float val, uint16_t *regs)
{
	uint32_t u32 = *(uint32_t *)&val;
	regs[0] = u32 >> 16;
	regs[1] = u32 & 0xffff;
}


static inline uint32_t uint32FromRegs(const uint16_t *regs)
{
	uint32_t val = (uint32_t)regs[0] << 16;
	val += (uint32_t)regs[1];
	return val;
}


static inline uint64_t uint64FromRegs(const uint16_t *regs)
{
	uint64_t val = (uint64_t)regs[0] << 48;
	val += (uint64_t)regs[1] << 32;
	val += (uint64_t)regs[2] << 16;
	val += (uint64_t)regs[3];
	return val;
}


static void stringFromRegs(const uint16_t *regs, char *buf, size_t bytesCount)
{
	for (size_t i = 0; i < bytesCount; i++) {
		if (i % 2 == 0) {
			buf[i] = (regs[i / 2]) >> 8 & 0xff;
		}
		else {
			buf[i] = regs[i / 2] & 0xff;
		}
	}
	buf[bytesCount] = '\0';
}


#if 0
static inline double doubleFromRegs(const uint16_t *regs)
{
	uint64_t u64 = (regs[0] << 48) + (regs[1] << 32) + (regs[0] << 16) + regs[3];
	return *(double *)(&u64);
}


static inline void doubleToRegs(double val, uint16_t *regs)
{
	uint64_t u32 = *(uint64_t *)&val;
	regs[0] = (u32 >> 48) & 0xffff;
	regs[1] = (u32 >> 32) & 0xffff;
	regs[2] = (u32 >> 16) & 0xffff;
	regs[3] = u32 & 0xffff;
}
#endif


static int getStorageInfo(devices_ctx_t *ctx, devices_storageInfo_t *info, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO, 14, regs);
	if (err != modbus_statusOk) {
		log_warn("reading storage regs failed %d", err);
		return -1;
	}

	/* TODO: change to doubles in the whole setup */
	info->maxCapacity = floatFromRegs(&regs[0]);
	info->minChargeLevel = floatFromRegs(&regs[2]);
	info->chargingSwitchLevel = floatFromRegs(&regs[4]);
	info->currChangeLevel = floatFromRegs(&regs[6]);
	info->nominalPower = floatFromRegs(&regs[8]);
	info->efficiency = floatFromRegs(&regs[10]);
	info->energyLoss = floatFromRegs(&regs[12]);

	return 0;
}


static int getEVInfo(devices_ctx_t *ctx, devices_EVInfo_t *info, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO, 17, regs);
	if (err != modbus_statusOk) {
		log_warn("reading ev regs failed %d", err);
		return -1;
	}

	info->maxCapacity = floatFromRegs(&regs[0]);
	info->minChargeLevel = floatFromRegs(&regs[2]);
	info->chargingSwitchLevel = floatFromRegs(&regs[4]);
	info->currChangeLevel = floatFromRegs(&regs[6]);
	info->nominalPower = floatFromRegs(&regs[8]);
	info->efficiency = floatFromRegs(&regs[10]);
	info->energyLoss = floatFromRegs(&regs[12]);
	info->drivingPower = floatFromRegs(&regs[14]);
	info->isAvailable = regs[16] > 0;

	return 0;
}


static int getHomeModelInfo(devices_ctx_t *ctx, devices_homeModelInfo_t *info, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO, 12, regs);
	if (err != modbus_statusOk) {
		log_warn("reading home model regs failed %d", err);
		return -1;
	}

	info->minTempSetting = floatFromRegs(&regs[0]);
	info->maxTempSetting = floatFromRegs(&regs[2]);
	info->heatingDeltaTemperature = floatFromRegs(&regs[4]);
	info->heatingCoeff = floatFromRegs(&regs[6]);
	info->heatLossCoeff = floatFromRegs(&regs[8]);
	info->heatCapacity = floatFromRegs(&regs[10]);
	return 0;
}


static int getHeatingInfo(devices_ctx_t *ctx, devices_heatingInfo_t *info, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;
	uint16_t regCnt = 5;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO, regCnt, regs);
	if (err != modbus_statusOk) {
		log_warn("reading heating regs failed %d", err);
		return -1;
	}

	info->currTemp = floatFromRegs(&regs[0]);
	info->optimalTemp = floatFromRegs(&regs[2]);

	info->heatDevCnt = regs[4];
	if (info->heatDevCnt > COGNIT_DEVICES_HEAT_DEV_MAX_CNT) {
		log_warn("Too many heating devices");
		return -1;
	}

	regCnt = info->heatDevCnt * 3;
	err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO + 5, regCnt, regs);
	if (err != modbus_statusOk) {
		log_warn("reading heating regs failed %d", err);
		return -1;
	}

	for (int i = 0; i < info->heatDevCnt; i++) {
		info->heatDevPwr[i] = floatFromRegs(&regs[2 * i]);
	}

	for (int i = 0; i < info->heatDevCnt; i++) {
		info->heatDevOn[i] = regs[2 * info->heatDevCnt + i] > 0;
	}

	return 0;
}


int devices_getInfo(devices_ctx_t *ctx, devices_info_t *info)
{
	if (devices_getMetersimInfo(ctx, &info->metersim) < 0) {
		log_warn("get metersim info failed");
		return -1;
	}

	for (uint8_t i = 0; i < ctx->simData.storageCount; i++) {
		if (getStorageInfo(ctx, &info->storage[i], (uint8_t)ADDRESS_FIRST_STORAGE + i) < 0) {
			log_warn("get storage %u info failed", i);
			return -1;
		}
	}

	for (uint8_t i = 0; i < ctx->simData.evCount; i++) {
		if (getEVInfo(ctx, &info->ev[i], ADDRESS_FIRST_EV + i) < 0) {
			log_warn("get EV %u info failed", i);
			return -1;
		}
	}

	for (uint8_t i = 0; i < ctx->simData.roomCount; i++) {
		if (getHeatingInfo(ctx, &info->heating[i], ADDRESS_FIRST_HEATING + i) < 0) {
			log_warn("get heating %u info failed", i);
			return -1;
		}
	}

	if (getHomeModelInfo(ctx, &info->homeModel, ADDRESS_HOME_MODEL) < 0) {
		log_warn("getHomeModelInfo failed");
		return -1;
	}

	return 0;
}


static int setStorageConfig(devices_ctx_t *ctx, const devices_storageParams_t *config, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	floatToRegs(config->inWRte, &regs[0]);
	floatToRegs(config->outWRte, &regs[2]);
	regs[4] = config->storCtl;

	modbus_status_t err = modbus_setMultiRegister(ctx->modbus, devAddr, REGISTER_CONFIG, 5, regs);
	if (err != modbus_statusOk) {
		log_warn("setting storage regs failed %d", err);
		return -1;
	}

	return 0;
}


static int setEVConfig(devices_ctx_t *ctx, const devices_EVParams_t *config, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	floatToRegs(config->inWRte, &regs[0]);
	floatToRegs(config->outWRte, &regs[2]);
	regs[4] = config->storCtl;

	modbus_status_t err = modbus_setMultiRegister(ctx->modbus, devAddr, REGISTER_CONFIG, 5, regs);
	if (err != modbus_statusOk) {
		log_warn("setting ev regs failed %d", err);
		return -1;
	}

	return 0;
}


static int setHeatingConfig(devices_ctx_t *ctx, const devices_heatingParams_t *config, uint8_t devAddr)
{
	uint16_t *regs = ctx->regsBuffer;

	floatToRegs(config->optimalTemp, &regs[0]);

	modbus_status_t err = modbus_setMultiRegister(ctx->modbus, devAddr, REGISTER_CONFIG, 2, regs);
	if (err != modbus_statusOk) {
		log_warn("setting heating regs failed %d", err);
		return -1;
	}

	return 0;
}


int devices_setParams(devices_ctx_t *ctx, const devices_config_t *config)
{
	bool failed = false;

	for (uint8_t i = 0; i < ctx->simData.storageCount; i++) {
		if (setStorageConfig(ctx, &config->storage[i], ADDRESS_FIRST_STORAGE + i) < 0) {
			log_warn("set storage %u params failed", i);
			failed = true;
		}
	}

	for (uint8_t i = 0; i < ctx->simData.evCount; i++) {
		if (setEVConfig(ctx, &config->ev[i], ADDRESS_FIRST_EV + i) < 0) {
			log_warn("set EV %u params failed", i);
			failed = true;
		}
	}

	for (uint8_t i = 0; i < ctx->simData.roomCount; i++) {
		if (setHeatingConfig(ctx, &config->heating[i], ADDRESS_FIRST_HEATING + i) < 0) {
			log_warn("set heating %u params failed", i);
			failed = true;
		}
	}

	return failed ? -1 : 0;
}


int devices_getMetersimInfo(devices_ctx_t *ctx, devices_metersimInfo_t *metersim)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, ADDRESS_METERSIM, REGISTER_INFO, 12, regs);
	if (err != modbus_statusOk) {
		log_warn("reading metersim regs failed %d", err);
		return -1;
	}

	metersim->aplus = uint64FromRegs(&regs[0]);
	metersim->aminus = uint64FromRegs(&regs[4]);
	metersim->timestamp = uint64FromRegs(&regs[8]);

	return 0;
}


int devices_getTimeMetersim(devices_ctx_t *ctx, uint64_t *val)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, ADDRESS_METERSIM, REGISTER_INFO + 8, 4, regs);
	if (err != modbus_statusOk) {
		log_warn("reading metersim time regs failed %d", err);
		return -1;
	}

	*val = uint64FromRegs(&regs[0]);

	log_debug("Metersim time: %llu", *val);
	return 0;
}


static int getUserPrefStr(devices_ctx_t *ctx, uint8_t devAddr, char *buf, size_t buflen)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO, 1, regs);
	if (err != modbus_statusOk) {
		log_warn("reading user pref length reg failed %d", err);
		return -1;
	}

	uint16_t length = regs[0];
	log_debug("Length: %u", length);
	if (length >= buflen) {
		return -1;
	}

	if (length == 0) {
		buf[0] = '\0';
		return 0;
	}

	size_t bytesRead = 0;
	while (bytesRead < length) {
		uint16_t regCnt = (length - bytesRead + 1) / 2;
		regCnt = regCnt <= MODBUS_MAX_REG_CNT ? regCnt : MODBUS_MAX_REG_CNT;
		err = modbus_getHoldingRegisters(ctx->modbus, devAddr, REGISTER_INFO + 1 + bytesRead / 2, regCnt, regs);
		if (err != modbus_statusOk) {
			log_warn("reading user pref regs failed %d", err);
			return -1;
		}
		bytesRead += regCnt * 2;
	}

	stringFromRegs(regs, buf, length);
	return length;
}


static int getOtherUserPref(devices_ctx_t *ctx, devices_userPref_t *pref)
{
	uint16_t *regs = ctx->regsBuffer;
	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, ADDRESS_SIMULATION_CONTROL, REGISTER_CONFIG, 5, regs);
	if (err != modbus_statusOk) {
		log_warn("reading other user pref reg failed %d", err);
		return -1;
	}

	pref->offloadFrequency = uint32FromRegs(&regs[1]);
	pref->trainingFrequency = uint32FromRegs(&regs[3]);
	return 0;
}


int devices_getUserPref(devices_ctx_t *ctx, devices_userPref_t *pref)
{
	if (getUserPrefStr(ctx, ADDRESS_EV_PREF, pref->ev, sizeof(pref->ev)) < 0) {
		log_warn("Getting EV pref failed");
		return -1;
	}
	log_debug("EV PREF: %s", pref->ev);

	if (getUserPrefStr(ctx, ADDRESS_HEATING_PREF, pref->temp, sizeof(pref->temp)) < 0) {
		log_warn("Getting EV pref failed");
		return -1;
	}
	log_debug("TEMP PREF: %s", pref->temp);

	if (getOtherUserPref(ctx, pref) < 0) {
		log_warn("Getting other user pref failed");
		return -1;
	}
	log_debug("Offload freq: %u", pref->offloadFrequency);

	log_debug("Updated user preferences");
	return 0;
}


static int getSimulationData(devices_ctx_t *ctx)
{
	uint16_t *regs = ctx->regsBuffer;

	modbus_status_t err = modbus_getHoldingRegisters(ctx->modbus, ADDRESS_SIMULATION_CONTROL, REGISTER_INFO, 5, regs);
	if (err != modbus_statusOk) {
		log_warn("reading sim control regs failed %d", err);
		return -1;
	}

	ctx->simData.pvCount = regs[0];
	ctx->simData.evCount = regs[1];
	ctx->simData.storageCount = regs[2];
	ctx->simData.roomCount = regs[3];
	ctx->simData.speedup = regs[4];

	return 0;
}


int devices_simulationSetTrainingState(devices_ctx_t *ctx, bool state)
{
	uint16_t *regs = ctx->regsBuffer;

	regs[0] = state ? 1 : 0;

	modbus_status_t err = modbus_setMultiRegister(ctx->modbus, ADDRESS_METERSIM, REGISTER_CONFIG, 1, regs);
	if (err != modbus_statusOk) {
		log_warn("setting training state regs failed %d", err);
		return -1;
	}

	return 0;
}


int devices_init(devices_ctx_t *ctx, modbus_t *modbus)
{
	ctx->modbus = modbus;

	if (getSimulationData(ctx) < 0) {
		return -1;
	}

	return 0;
}
