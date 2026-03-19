#!/usr/bin/env bash
#
# Phoenix-RTOS
#
# Shell script for running Phoenix-RTOS on armv7r5f-zynqmp-qemu
# Can be sourced to reuse QEMU_ARGS in other scripts
#
# Copyright 2025, 2026 Phoenix Systems
# Author: Jacek Maksymowicz, Damian Loewnau
#
# SPDX-License-Identifier: BSD-3-Clause
#

IMG_PLO_ZYNQMP="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7r5f-zynqmp-qemu/plo.elf"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7r5f-zynqmp-qemu/flash0.disk"
DTB_ZYNQMP="$(dirname "${BASH_SOURCE[0]}")/../scripts/zynqmp-zcu104.dtb"
QEMU_ARGS=(
	-M arm-generic-fdt
	-serial null
	-serial mon:stdio
	-hw-dtb "$DTB_ZYNQMP"
	-device "loader,file=$IMG_PLO_ZYNQMP,cpu-num=4"
	-drive "file=$IMG_FLASH_QEMU,if=mtd,format=raw,index=0"
	-device "loader,addr=0xff5e023c,data=0x80008fde,data-len=4"
	-m 2G
	-display none
)

for FILE in "$IMG_PLO_ZYNQMP" "$IMG_FLASH_QEMU" "$DTB_ZYNQMP"; do
	if [ ! -f "$FILE" ]; then
		echo "Missing required file: $FILE"
		# Works both when sourced (return) and run directly (exit)
		# shellcheck disable=SC2317
		return 1 2>/dev/null || exit 1
	fi
done

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
	exec qemu-system-aarch64 "${QEMU_ARGS[@]}"
fi
