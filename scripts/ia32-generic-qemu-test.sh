#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU for testing purposes (ia32-generic-qemu)
#
# Copyright 2022 Phoenix Systems
# Author: Damian Loewnau
#

BOOT_DIR="${PHOENIX_BOOT_DIR:-$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu}"

exec qemu-system-i386 \
	-cpu pentium3 \
	-drive "file=$BOOT_DIR/hd0.disk,format=raw,media=disk,index=0" \
	-nographic \
	-monitor none \
	-netdev user,id=net0 -device rtl8139,netdev=net0 "$@" \
	-d cpu_reset
