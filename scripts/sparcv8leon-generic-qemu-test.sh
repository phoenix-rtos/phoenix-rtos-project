#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on sparcv8leon-generic-qemu for test purposes.
#
# Copyright 2024 Phoenix Systems
# Author: Lukasz Leczkowski, Damian Modzelewski
#

BOOT_DIR="${PHOENIX_BOOT_DIR:-$(dirname "${BASH_SOURCE[0]}")/../_boot/sparcv8leon-generic-qemu}"

IMG_PLO_LEON3="$BOOT_DIR/plo.elf"
IMG_FLASH_QEMU="$BOOT_DIR/phoenix.disk"

exec qemu-system-sparc \
	-smp 1 \
	-m 128M \
	-M leon3_generic \
	-serial stdio \
	-nographic \
	-monitor none \
	-kernel "$IMG_PLO_LEON3" \
	-device loader,file="$IMG_FLASH_QEMU",addr=0x47e00000,force-raw=on
