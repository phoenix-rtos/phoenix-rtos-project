#!/bin/bash
#
# Shell script for running the devel docker with arguments passed to the script
#
# Copyright 2021, 2023 Phoenix Systems
# Author: Hubert Buczynski, Hubert Badocha
#

DOCKER_IMG_NAME=maska989/devel:amd64
PATH_TO_PROJECT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")/"

if [ "$#" -eq 0 ]; then
    exec docker run -it --privileged --rm -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project" -w /project $DOCKER_IMG_NAME /usr/bin/entrypoint.sh
else
    docker run -it --privileged --rm -v /dev/bus/usb/:/dev/bus/usb -v "${PATH_TO_PROJECT}:/project:delegated" -w /project $DOCKER_IMG_NAME "$@"
fi
