#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (riscv64-generic-spike)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

# TODO: this script should run spike simulator

REGEX="version ([0-9]+)\.([0-9]+)"
OPTIMG=""

# Determine QEMU version
[[ $(qemu-system-riscv64 --version) =~ $REGEX ]]
MAJOR="${BASH_REMATCH[1]}"
MINOR="${BASH_REMATCH[2]}"

# Select QEMU option for passing kernel image
if [[ "$MAJOR" -gt 4 || ( "$MAJOR" -eq 4 && "$MINOR" -ge 1 ) ]]; then
	OPTIMG="-bios"
else
	OPTIMG="-kernel"
fi

exec qemu-system-riscv64 \
	-smp 1 \
	-machine virt \
	"$OPTIMG" "$(dirname "${BASH_SOURCE[0]}")/../_boot/riscv64-generic-spike/phoenix.bbl" \
	-serial stdio \
	-device virtio-gpu-device \
	-drive file="$(dirname "${BASH_SOURCE[0]}")/../_boot/riscv64-generic-spike/phoenix.disk",format=raw,cache=unsafe,if=none,id=vblk0 \
	-device virtio-blk-device,drive=vblk0 \
	-netdev user,id=net0 -device virtio-net-device,netdev=net0
