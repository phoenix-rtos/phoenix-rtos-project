#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached VirtIO network card device (riscv64-virt)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

if [ $# -eq 0 ]; then
	NETDEVS=("-vnet" "user")
else
	NETDEVS=("-vnet" "$@")
fi

exec bash "$(dirname "${BASH_SOURCE[0]}")/riscv64-virt.sh" "${NETDEVS[@]}"
