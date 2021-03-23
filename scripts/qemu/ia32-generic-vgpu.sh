#!/bin/bash
#
# Shell script for running Phoenix-RTOS on QEMU with attached VirtIO GPU device (ia32-generic)
#
# Copyright 2021 Phoenix Systems
# Author: Lukasz Kosinski
#

if [ $# -gt 1 ]; then
	echo "Usage: $0 [outputs]"
	exit 1
fi

exec bash "$(dirname "${BASH_SOURCE[0]}")/ia32-generic.sh" "-vgpu" "$@"
