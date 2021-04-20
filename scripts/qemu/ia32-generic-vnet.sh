#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached VirtIO network card device (ia32-generic)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

if [ $# -eq 0 ]; then
	NETDEVS=("-vnet" "user")
else
	NETDEVS=("-vnet" "$@")
fi

exec bash "$(dirname "${BASH_SOURCE[0]}")/ia32-generic.sh" "${NETDEVS[@]}"
