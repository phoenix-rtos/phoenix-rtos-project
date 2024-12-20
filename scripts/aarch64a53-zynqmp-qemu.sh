#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on aarch64a53-zynqmp-qemu
#
# Copyright 2024 Phoenix Systems
# Author: Jacek Maksymowicz
#

IMG_PLO_ZYNQMP="$(dirname "${BASH_SOURCE[0]}")/../_boot/aarch64a53-zynqmp-qemu/plo.elf"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/aarch64a53-zynqmp-qemu/flash0.disk"
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
	-kernel "$IMG_PLO_ZYNQMP" \
	-drive file="$IMG_FLASH_QEMU",if=mtd,format=raw,index=0 \
	-device loader,addr=0xfd1a0104,data=0x80000e0e,data-len=4 \
	-m 2G \
	-display none
