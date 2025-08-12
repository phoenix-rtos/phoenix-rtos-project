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

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "modbus_comm.h"


#define COGNIT_APP_LOG_TAG "cognit_app/modbusComm : "
#define COGNIT_APP_LOG_LVL COGNIT_APP_LOG_LVL_DEBUG
#include "logger.h"


#define READ_WRITE_SLEEP_US (20 * 1000)


static struct {
	bool serialInitialized;
	bool tcpInitialized;
	int fd;
} common;


static uint64_t getTimeMonoMs(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / (1000 * 1000);
}


int modbusComm_read(uint8_t *buf, size_t buflen, size_t bytesToRead, uint32_t read_timeout_ms, void *arg)
{
	(void)arg;
	size_t bytesRead = 0;

	uint64_t startTime = getTimeMonoMs();
	while (1) {
		int ret = read(common.fd, buf + bytesRead, buflen - bytesRead);
		if (ret < 0 && errno != EWOULDBLOCK) {
			return -1;
		}

		if (ret > 0) {
			bytesRead += ret;
			continue;
		}

		if (bytesRead >= bytesToRead) {
			break;
		}

		uint64_t now = getTimeMonoMs();
		if (now >= startTime + read_timeout_ms) {
			break;
		}

		usleep(READ_WRITE_SLEEP_US);
	}

	return (int)bytesRead;
}


int modbusComm_write(const uint8_t *buf, size_t count, uint32_t byte_timeout_ms, void *arg)
{
	(void)arg;
	size_t bytesWritten = 0;

	uint64_t startTime = getTimeMonoMs();
	while (1) {
		int ret = write(common.fd, buf + bytesWritten, count - bytesWritten);
		if (ret < 0 && errno != EWOULDBLOCK) {
			return -1;
		}

		if (ret > 0) {
			bytesWritten += ret;
		}

		if (bytesWritten == count) {
			break;
		}

		uint64_t now = getTimeMonoMs();
		if (now >= startTime + byte_timeout_ms) {
			break;
		}

		usleep(READ_WRITE_SLEEP_US);
	}

	return (int)bytesWritten;
}


static int setFrameFormat(int fd, uint32_t baudrate)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		log_error("tcgetattr");
		return -1;
	}
	cfmakeraw(&tty);
	cfsetospeed(&tty, B9600); /* TODO: set baudrate */
	cfsetispeed(&tty, B9600);
	tty.c_iflag &= ~INPCK;
	tty.c_cflag |= CSIZE | CS8;        /* 8-bit chars */
	tty.c_cflag &= ~(PARENB | PARODD); /* shut off parity */
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		log_error("tcsetattr");
		return -1;
	}

	return 0;
}


void modbusComm_drain(void)
{
	uint8_t buf[8];
	while (read(common.fd, buf, sizeof(buf)) > 0) { }
}


int modbusComm_initSerial(const char *dev, uint32_t baudrate)
{
	if (common.serialInitialized || common.tcpInitialized) {
		return -1;
	}

	common.fd = open(dev, O_RDWR | O_NONBLOCK);
	if (common.fd < 0) {
		log_error("Could not open serial dev");
		return -1;
	}

	if (setFrameFormat(common.fd, baudrate) < 0) {
		return -1;
	}

	modbusComm_drain();

	common.serialInitialized = true;
	return 0;
}


int modbusComm_initTcp(const char *address, const char *service)
{
	if (common.serialInitialized || common.tcpInitialized) {
		return -1;
	}

	common.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (common.fd < 0) {
		log_error("Could not initialize socket");
		return -1;
	}

	const struct addrinfo hints = { 0 };
	struct addrinfo *addr;
	log_debug("Addr: %s, port: %s", address, service);
	int ret = getaddrinfo(address, service, &hints, &addr);
	if (ret < 0) {
		log_error("getaddrinfo failed %s", gai_strerror(ret));
		freeaddrinfo(addr);
		return -1;
	}

	ret = connect(common.fd, addr->ai_addr, addr->ai_addrlen);
	freeaddrinfo(addr);
	if (ret < 0) {
		log_error("connect failed");
		return -1;
	}

	int flags = fcntl(common.fd, F_GETFL, 0);
	if (flags == -1) {
		log_error("fcntl get failed");
		return -1;
	}
	flags |= O_NONBLOCK;
	ret = fcntl(common.fd, F_SETFL, flags);
	if (ret < 0) {
		log_error("fcntl set failed");
		return -1;
	}

	common.tcpInitialized = true;
	return 0;
}


int modbusComm_deinit(void)
{
	common.serialInitialized = false;
	common.tcpInitialized = false;
	close(common.fd);
	common.fd = -1;
	return 0;
}
