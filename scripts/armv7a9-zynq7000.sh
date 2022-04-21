#!/bin/bash
#
# Shell script for running phoenix-rtos loader on Qemu fork from Xilinx (zynq-7000)
#
# Copyright 2021 Phoenix Systems
# Author: Hubert Buczynski
#

IMG_PLO_ZYNQ7000="$(dirname "${BASH_SOURCE[0]}")/../_boot/plo-armv7a9-zynq7000-qemu.img"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/flash-armv7a9-zynq7000-qemu.bin"
DTB_ZYNQ7000="$(dirname "${BASH_SOURCE[0]}")/../scripts/zynq7000-zc702.dtb"

for FILE in "$IMG_PLO_ZYNQ7000" "$IMG_FLASH_QEMU" "$DTB_ZYNQ7000"; do
	if [ ! -f "$FILE" ]; then
		echo "Missing required file: $FILE"
		exit 1
	fi
done

exec qemu-system-aarch64 \
	-M arm-generic-fdt-7series \
	-dtb "$DTB_ZYNQ7000" \
	-serial null \
	-serial mon:stdio \
	-device loader,file="$IMG_PLO_ZYNQ7000" \
	-drive file="$IMG_FLASH_QEMU",if=mtd,format=raw,index=0
