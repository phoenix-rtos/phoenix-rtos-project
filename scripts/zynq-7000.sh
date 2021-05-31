#!/bin/bash
#
# Shell script for running phoenix-rtos loader on QEMU (zynq-7000)
#
# Copyright 2021 Phoenix Systems
# Author: Hubert Buczynski
#

IMG_ZYNQ7000="$(dirname "${BASH_SOURCE[0]}")/../_boot/plo-ram-armv7a9-zynq7000.img"

if [ ! -f "$IMG_ZYNQ7000" ]; then
	echo "File $IMG_ZYNQ7000 does not exist"
	exit 1
fi

exec qemu-system-arm \
	-M xilinx-zynq-a9 \
	-serial null \
	-serial mon:stdio \
	-device loader,file="$IMG_ZYNQ7000"
