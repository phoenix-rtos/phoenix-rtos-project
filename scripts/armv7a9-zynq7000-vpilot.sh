#!/bin/bash
#
# Shell script for running pilot project loader on Qemu fork from Xilinx (zynq-7000)
#
# Copyright 2023 Phoenix Systems
# Author: Piotr Nieciecki
#

IMG_PLO_ZYNQ7000="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7a9-zynq7000-vpilot/plo.img"
IMG_FLASH_QEMU="$(dirname "${BASH_SOURCE[0]}")/../_boot/armv7a9-zynq7000-vpilot/phoenix.disk"
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
