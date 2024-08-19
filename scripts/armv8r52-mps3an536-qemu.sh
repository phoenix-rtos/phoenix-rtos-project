#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on armv8r52-mps3an536-qemu
#
# Copyright 2024 Phoenix Systems
# Author: Lukasz Leczkowski
#

IMG_PLO_ARMv8r52="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv8r52-mps3an536-qemu/plo.elf"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv8r52-mps3an536-qemu/phoenix.disk"

# uses UART2 for output
exec qemu-system-arm \
	-smp 1 \
	-M mps3-an536 \
	-serial null \
	-serial null \
	-serial mon:stdio \
	-kernel "$IMG_PLO_ARMv8r52" \
	-device loader,file="$IMG_FLASH_QEMU",addr=0x08000000,force-raw=on
