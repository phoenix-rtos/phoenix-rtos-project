#!/usr/bin/env bash
#
# Shell script for plo setup without flashing the image
#
# Copyright 2021, 2023 Phoenix Systems
# Author: Aleksander Kaminski, Gerard Swiderski, Hubert Badocha
#
set -e

cd "$(dirname "$0")"

TARGET=armv7a7-imx6ull-evk

. flashing-common.bash

check_arguments "$@"

pushd "${boot_path}/${TARGET}"
	./psu plo-ram.sdp
	run_phoenixd "${prog_path}"
    sleep 1
popd

echo "Sending plo commands:"

plo_command "kernel usb0"
plo_command "app usb0 -x dummyfs ddr ddr"
plo_command "app usb0 -x imx6ull-uart ddr ddr"
plo_command "app usb0 -x psh ddr ddr"
plo_command "go!"
