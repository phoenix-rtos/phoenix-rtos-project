#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on armv7r5f-zynqmp-qemu
#
# Copyright 2025 Phoenix Systems
# Author: Jacek Maksymowicz
#

IMG_PLO_ZYNQMP="$(dirname "${BASH_SOURCE[0]}")/../_build/armv7r5f-zynqmp-qemu/prog/plo-armv7r5f-zynqmp.elf"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7r5f-zynqmp-qemu/flash0.disk"
DTB_ZYNQMP="$(dirname "${BASH_SOURCE[0]}")/../scripts/zynqmp-zcu104.dtb"

for FILE in "$IMG_PLO_ZYNQMP" "$IMG_FLASH_QEMU" "$DTB_ZYNQMP"; do
	if [ ! -f "$FILE" ]; then
		echo "Missing required file: $FILE"
		exit 1
	fi
done

exec qemu-system-aarch64 \
	-M arm-generic-fdt \
	-serial null \
	-serial mon:stdio \
	-hw-dtb "$DTB_ZYNQMP" \
	-device loader,file="$IMG_PLO_ZYNQMP",cpu-num=4 \
	-drive file="$IMG_FLASH_QEMU",if=mtd,format=raw,index=0 \
	-device loader,addr=0xff5e023c,data=0x80008fde,data-len=4 \
	-m 2G \
	-display none
