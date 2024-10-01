#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on sparcv8leon-generic-qemu
#
# Copyright 2024 Phoenix Systems
# Author: Lukasz Leczkowski
#

IMG_PLO_LEON3="$(dirname "${BASH_SOURCE[0]}")/../_boot/sparcv8leon-generic-qemu/plo.elf"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/sparcv8leon-generic-qemu/phoenix.disk"

exec qemu-system-sparc \
	-smp 1 \
	-m 128M \
	-M leon3_generic \
	-serial mon:stdio \
	-kernel "$IMG_PLO_LEON3" \
	-device loader,file="$IMG_FLASH_QEMU",addr=0x47e00000,force-raw=on
