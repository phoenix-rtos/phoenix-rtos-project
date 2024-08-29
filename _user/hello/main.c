/*
 * Phoenix-RTOS
 *
 * Hello World
 *
 * Example of user application
 *
 * Copyright 2021 Phoenix Systems
 * Author: Hubert Buczynski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#include <grlib-multi.h>

static oid_t getOid(const char *path)
{
	oid_t oid;
	while (lookup(path, NULL, &oid) < 0) {
		usleep(100000);
	}

	return oid;
}


static unsigned int spwConfigureRx(const oid_t rxOid, const size_t nPackets)
{
	/* Configure RX on SPW0 */
	msg_t msg = {
		.type = mtDevCtl,
		.i = { .data = NULL, .size = 0 },
		.o = { .data = NULL, .size = 0 },
		.oid.id = id_spw0,
		.oid.port = rxOid.port,
	};

	multi_i_t *idevctl = (multi_i_t *)msg.i.raw;
	multi_o_t *odevctl = (multi_o_t *)msg.o.raw;

	idevctl->spw.type = spw_rxConfig;
	idevctl->spw.task.rxConfig.nPackets = nPackets;

	if (msgSend(rxOid.port, &msg) < 0) {
		printf("msgSend failed\n");
		return 1;
	}

	if (nPackets != msg.o.err) {
		printf("nPackets != msg.o.err\n");
		return 1;
	}

	return odevctl->val;
}


static int spwRxRead(const oid_t rxOid, const unsigned int firstDesc, uint8_t *rxBuf, size_t rxBufsz, spw_rxPacket_t *packets, const size_t nPackets)
{
	msg_t msg = {
		.type = mtDevCtl,
		.i = { .data = NULL, .size = 0 },
		.o = { .data = rxBuf, .size = rxBufsz },
		.oid.id = id_spw0,
		.oid.port = rxOid.port,
	};
	multi_i_t *idevctl = (multi_i_t *)msg.i.raw;

	idevctl->spw.type = spw_rx;
	idevctl->spw.task.rx.nPackets = nPackets;
	idevctl->spw.task.rx.firstDesc = firstDesc;

	if (msgSend(rxOid.port, &msg) < 0) {
		printf("msgSend failed\n");
		return -1;
	}

	if (nPackets != msg.o.err) {
		printf("nPackets != msg.o.err\n");
		return -1;
	}

	for (size_t i = 0; i < nPackets; i++) {
		rxBuf += multi_spwDeserializeRxMsg(rxBuf, &packets[i]);
	}

	return 0;
}


static int spwTx(const oid_t txOid, uint8_t *txBuf, const size_t txBufsz, const size_t nPackets, bool async)
{
	msg_t msg = {
		.type = mtDevCtl,
		.i = { .data = txBuf, .size = txBufsz },
		.o = { .data = NULL, .size = 0 },
		.oid.id = id_spw0,
		.oid.port = txOid.port,
	};

	multi_i_t *idevctl = (multi_i_t *)msg.i.raw;

	idevctl->spw.type = spw_tx;
	idevctl->spw.task.tx.nPackets = nPackets;
	idevctl->spw.task.tx.async = true;

	if (msgSend(txOid.port, &msg) < 0) {
		printf("msgSend failed\n");
		return -1;
	}
	if (nPackets != msg.o.err) {
		printf("nPackets != msg.o.err\n");
		return -1;
	}

	return 0;
}


static int response(uint8_t command, oid_t rxOid, uint8_t *buf, size_t bufsz)
{
	usleep(100);

	/* Send response */
	static const uint8_t hdr[] = { 0x00, 0x05 };
	uint8_t data[3];
	uint16_t val = 0;
	data[0] = command;
	switch (command) {
		case 0x1:
			val = 1805 + rand() % 10;
			break;

		case 0x2:
			val = 3300 + (rand() % 10);
			break;

		case 0x3:
			val = 320 + rand() % 50;
			break;

		case 0x4:
			val = 300 + rand() % 50;
			break;
	}

	data[1] = val >> 8;
	data[2] = val & 0xff;

	multi_spwSerializeTxMsg(SPW_TX_FLG_HDR_LEN(2), sizeof(data), hdr, data, buf, bufsz);
	if (spwTx(rxOid, buf, bufsz, 1, true) < 0) {
		printf("spwTx failed\n");
		return -1;
	}

	return 0;
}


static void spwLoop(void)
{
	oid_t rxOid = getOid("/dev/spw0");

	/* Receive packet */
	const size_t bufsz = (SPW_RX_MIN_BUFSZ + 60);
	uint8_t *buf = malloc(bufsz);
	if (buf == NULL) {
		printf("malloc failed\n");
		return;
	}

	uint8_t command = 0;
	while (command != 0x5) {
		unsigned int firstDesc = spwConfigureRx(rxOid, 1);
		spw_rxPacket_t packet;
		if (spwRxRead(rxOid, firstDesc, buf, bufsz, &packet, 1) < 0) {
			printf("spwRxRead failed\n");
			break;
		}
		if ((packet.buf[0] != 0x00) && (packet.buf[1] != 0x05)) {
			printf("Received packet with wrong protocol ID\n");
			continue;
		}
		/* Skip header of len 2 bytes */
		command = packet.buf[2];
		switch (command) {
			case 0x1:
				printf("CPU voltage read\n");
				break;
			case 0x2:
				printf("Peripheral voltage read\n");
				break;
			case 0x3:
				printf("CPU temperature read\n");
				break;
			case 0x4:
				printf("FPGA temperature read\n");
				break;
			case 0x5:
				printf("Closing connection\n");
				break;
			default:
				printf("Received unknown command\n");
				break;
		}

		if (command > 0 && command < 0x5) {
			if (response(command, rxOid, buf, bufsz) < 0) {
				break;
			}
		}
	}

	free(buf);
}


int main(int argc, char *argv[])
{
	srand(time(NULL));
	printf("Welcome to SpaceWire Demo!\n");
	printf("Waiting for commands...\n");
	spwLoop();

	return 0;
}
