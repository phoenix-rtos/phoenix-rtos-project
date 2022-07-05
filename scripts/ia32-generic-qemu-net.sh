#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with vibr0 bridge (ia32-generic-qemu)
#
# Copyright 2022 Phoenix Systems
# Author: Damian Loewnau
#

exec qemu-system-i386 \
	-cpu pentium3 \
	-smp 1 \
	-serial stdio \
	-vga cirrus \
	-hda "$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/phoenix.disk" \
	-netdev bridge,br=virbr0,id=net0 -device rtl8139,netdev=net0,id=nic0,addr=03.0
