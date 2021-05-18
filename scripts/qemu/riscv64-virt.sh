#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (riscv64-virt)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

exec qemu-system-riscv64 \
	-smp 2 \
	-machine virt \
	-bios "$(dirname "${BASH_SOURCE[0]}")/../../_boot/phoenix-riscv64-virt.osbi" \
	-serial stdio \
	-device virtio-gpu-device \
	-drive file="$(dirname "${BASH_SOURCE[0]}")/../../_boot/phoenix-riscv64-virt.disk",cache=unsafe,if=virtio \
	-netdev user,id=net0 -device virtio-net-device,netdev=net0
