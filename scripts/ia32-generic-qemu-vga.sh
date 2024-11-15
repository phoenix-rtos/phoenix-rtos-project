#!/bin/bash

exec qemu-system-i386 \
	-cpu pentium3 \
	-smp 4 \
	-serial stdio \
	-vga std \
	-drive "file=$(dirname "${BASH_SOURCE[0]}")/../_boot/ia32-generic-qemu/hd0.disk,format=raw,media=disk,index=0" \
	-netdev user,id=net0 -device rtl8139,netdev=net0 "$@"
