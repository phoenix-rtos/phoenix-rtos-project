#!/bin/bash
#
# Shell script for loading plo and launching RTT on STM32N6 devices.
# It may be used for flashing or for running applications over RTT.
#
# Copyright 2026 Phoenix Systems
# Author: Jacek Maksymowicz
#

set -e

USAGE="Usage: $0 {flash,run} <project_name>"

PROJECT_NAME=$2
PROG_PATH="_build/$PROJECT_NAME/prog.stripped"
BOOT_PATH="_boot/$PROJECT_NAME"

if [[ -z $PROJECT_NAME ]]; then
	echo "$USAGE"
	exit 1
elif [[ ! $PROJECT_NAME =~ "armv8m55-stm32n6" ]]; then
	echo "WARNING: project name $PROJECT_NAME is likely incorrect"
fi

if [[ $1 == "flash" ]]; then
	PHOENIXD_TARGET=$BOOT_PATH
	export PLO_PATH="$BOOT_PATH/plo-ram.elf"
elif [[ $1 == "run" ]]; then
	PHOENIXD_TARGET=$PROG_PATH
	export PLO_PATH="$BOOT_PATH/plo.elf"
else
	echo "$USAGE"
	exit 1
fi

openocd \
	-f interface/stlink.cfg \
	-f scripts/openocd/stm32n6/stm32n6x.cfg \
	-c "adapter speed 24000" \
	-f scripts/stm32n6-plo-rtt.cfg \
	&

background_pid=$!

cleanup() {
	kill -9 "$background_pid" 2>/dev/null || true
	wait "$background_pid" 2>/dev/null || true
}

trap cleanup EXIT INT TERM HUP
sleep 1.5 # Wait for openocd to open RTT channel
"$BOOT_PATH"/phoenixd -s "$PHOENIXD_TARGET" -t 127.0.0.1:18022
