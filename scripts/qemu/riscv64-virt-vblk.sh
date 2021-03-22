#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached VirtIO drives (riscv64-virt)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

# Add VirtIO drives
while [ "$#" -gt 0 ]; do
	DRIVES+=("-vblk" "$1")
	shift
done

exec bash "$(dirname "${BASH_SOURCE[0]}")/riscv64-virt.sh" "${DRIVES[@]}"
