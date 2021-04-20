#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached ATA drives (ia32-generic)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

DRIVES=()

# Add ATA drives
while [ $# -gt 0 ]; do
	DRIVES+=("-ata" "$1")
	shift
done

exec bash "$(dirname "${BASH_SOURCE[0]}")/ia32-generic.sh" "${DRIVES[@]}"
