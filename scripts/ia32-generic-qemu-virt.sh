#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with VirtIO devices (ia32-generic-qemu)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

exec qemu-system-i386 \
	-cpu pentium3 \
	-smp 1 \
	-serial stdio \
	-device virtio-gpu-pci \
	-hda "$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/phoenix.disk" \
	-netdev user,id=net0 -device virtio-net-pci,netdev=net0
