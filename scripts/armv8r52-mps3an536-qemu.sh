#!/usr/bin/env bash
#
# Shell script for running Phoenix-RTOS on armv8r52-mps3an536-qemu
#
# Copyright 2024 Phoenix Systems
# Author: Lukasz Leczkowski
#

# uses UART2 for output
exec qemu-system-arm \
	-smp 1 \
	-M mps3-an536 \
	-serial null \
	-serial null \
	-serial mon:stdio \
	-kernel _build/armv8r52-mps3an536-qemu/prog.stripped/plo-armv8r52-mps3an536.elf \
	-device loader,file=_boot/armv8r52-mps3an536-qemu/phoenix.disk,addr=0x08000000,force-raw=on
