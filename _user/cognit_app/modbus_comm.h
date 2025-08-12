/*
 * Phoenix-RTOS
 *
 * Helpers for modbus communication interfaces
 *
 * Copyright 2025 Phoenix Systems
 * Author: Mateusz Kobak
 *
 * %LICENSE%
 */

#ifndef COGNIT_MODBUS_COMM_H
#define COGNIT_MODBUS_COMM_H

#include <stddef.h>
#include <stdint.h>


int modbusComm_read(uint8_t *buf, size_t buflen, size_t bytesToRead, uint32_t read_timeout_ms, void *arg);


int modbusComm_write(const uint8_t *buf, size_t count, uint32_t byte_timeout_ms, void *arg);


void modbusComm_drain(void);


int modbusComm_initSerial(const char *name, uint32_t baudrate);


int modbusComm_initTcp(const char *port, const char *service);


int modbusComm_deinit(void);


#endif /* COGNIT_MODBUS_COMM_H */
