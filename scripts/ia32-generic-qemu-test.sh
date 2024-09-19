#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU for testing purposes (ia32-generic-qemu)
#
# Copyright 2022 Phoenix Systems
# Author: Damian Loewnau
#

exec qemu-system-i386 \
	-cpu pentium3 \
	-drive "file=$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/hd0.disk,format=raw,media=disk,index=0" \
	-nographic \
	-monitor none \
	-netdev user,id=net0 -device rtl8139,netdev=net0 "$@" \
	-d cpu_reset
