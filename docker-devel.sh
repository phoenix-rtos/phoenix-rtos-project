#!/bin/bash
#
# Shell script for running the devel docker with arguments passed to the script
#
# Copyright 2021, 2023 Phoenix Systems
# Author: Hubert Buczynski, Hubert Badocha
#

DOCKER_IMG_NAME=phoenixrtos/devel
PATH_TO_PROJECT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")/"

if [ "$#" -eq 0 ]; then
    exec docker run -it --privileged --rm -h "$(hostname)-docker" --env-file .docker_env -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project" -w /project "$DOCKER_IMG_NAME" bash
else
    exec docker run -it --privileged --rm -h "$(hostname)-docker" --env-file .docker_env -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project:delegated" -w /project "$DOCKER_IMG_NAME" "$(printf "%q " "$@")"
fi
