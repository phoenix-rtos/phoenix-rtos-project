#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached Realtek RTL8139 network card (ia32-generic)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

if [ "$#" -eq 0 ]; then
	NETDEVS=("-rtl" "user")
else
	NETDEVS=("-rtl" "$@")
fi

exec bash "$(dirname "${BASH_SOURCE[0]}")/ia32-generic.sh" "${NETDEVS[@]}"
