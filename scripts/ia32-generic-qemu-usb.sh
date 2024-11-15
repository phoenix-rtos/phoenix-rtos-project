#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU (ia32-generic-qemu)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

exec qemu-system-i386 \
	-cpu pentium3 \
	-smp 2 \
  -serial stdio \
	-drive "file=$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/hd0.disk,format=raw,media=disk,index=0" \
	-netdev user,id=net0 -device rtl8139,netdev=net0 "$@" \
  -drive if=none,id=usbstick,format=raw,"file=$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/hd0-copy.disk,format=raw,media=disk,index=0" \
  -device usb-ehci,id=ehci1                                  \
  -device usb-ehci,id=ehci2                                  \
  -device usb-storage,bus=ehci1.0,id=testdrive,drive=usbstick \
	-vga cirrus 
  # -display none \
