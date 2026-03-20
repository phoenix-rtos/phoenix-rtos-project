#!/usr/bin/env bash
#
# Phoenix-RTOS
#
# Shell script for running Phoenix-RTOS on armv7r5f-zynqmp-qemu for testing
# Sources armv7r5f-zynqmp-qemu.sh and extends QEMU_ARGS with a GDB server
#
# Copyright 2026 Phoenix Systems
# Author: Damian Loewnau
#
# SPDX-License-Identifier: BSD-3-Clause
#

# shellcheck source=scripts/armv7r5f-zynqmp-qemu.sh
source "$(dirname "${BASH_SOURCE[0]}")/armv7r5f-zynqmp-qemu.sh" || exit 1

QEMU_ARGS+=("-gdb" "tcp::2024")

exec qemu-system-aarch64 "${QEMU_ARGS[@]}"
