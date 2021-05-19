#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (ia32-virt)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

exec qemu-system-i386 \
	-smp 2 \
	-serial stdio \
	-device virtio-gpu-pci \
	-drive file="$(dirname "${BASH_SOURCE[0]}")/../_boot/phoenix-ia32-generic.disk",cache=unsafe,if=virtio \
	-netdev user,id=net0 -device virtio-net-pci,netdev=net0
