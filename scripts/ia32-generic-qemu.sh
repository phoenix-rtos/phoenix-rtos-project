#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (ia32-generic-qemu)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

exec qemu-system-i386 \
	-cpu pentium3 \
	-smp 1 \
	-serial stdio \
	-vga cirrus \
	-drive "file=$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/phoenix.disk,format=raw,media=disk,index=0" \
	-netdev user,id=net0 -device rtl8139,netdev=net0 "$@"
