#!/bin/bash
#
# Shell script for running the devel docker with arguments passed to the script
#
# Copyright 2021, 2023 Phoenix Systems
# Author: Hubert Buczynski, Hubert Badocha
#

DOCKER_IMG_NAME=phoenixrtos/devel
PATH_TO_PROJECT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")/"

RUST_ARGS=()
if [ "$RUST_ENABLE" = "y" ] && [ "$*" != "pull" ]; then
    if [ -n "$RUST_TOOLCHAIN_MOUNT" ]; then
        RUST_TOOLCHAIN_HOST="$(realpath "$RUST_TOOLCHAIN_MOUNT")"
    else
        RUST_TOOLCHAIN_HOST="$("${PATH_TO_PROJECT}scripts/rust-toolchain.sh")" || exit 1
    fi
    RUST_ARGS=(-v "${RUST_TOOLCHAIN_HOST}:/opt/phoenix/rust:ro" -e RUST_TOOLCHAIN_DIR=/opt/phoenix/rust -e CARGO_HOME=/project/_build/.cargo-home -e RUST_RUNNER_DOCKER=0)
fi

if [ "$#" -eq 0 ]; then
    exec docker run -it --privileged --rm -h "$(hostname)-docker" --env-file .docker_env "${RUST_ARGS[@]}" -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project" -w /project "$DOCKER_IMG_NAME" bash
elif [ "$#" -eq 1 ] && [ "$1" = "pull" ]; then
    # explicitly pull (update local image)
    exec docker pull "$DOCKER_IMG_NAME"
else
    exec docker run -it --privileged --rm -h "$(hostname)-docker" --env-file .docker_env "${RUST_ARGS[@]}" -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project:delegated" -w /project "$DOCKER_IMG_NAME" "$(printf "%q " "$@")"
fi
