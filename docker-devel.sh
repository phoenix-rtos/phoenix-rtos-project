#!/bin/bash
#
# Shell script for running the devel docker with arguments passed to the script
#
# Copyright 2021 Phoenix Systems
# Author: Hubert Buczynski
#

DOCKER_IMG_NAME=phoenixrtos/devel
PATH_TO_PROJECT="$(dirname "$(pwd)/${BASH_SOURCE[0]}")/"

if [ "$#" -eq 0 ]; then
    exec docker run -it --rm -v "${PATH_TO_PROJECT}:/project" -w /project $DOCKER_IMG_NAME bash
else
    docker run -it --rm -v "${PATH_TO_PROJECT}:/project:delegated" -w /project $DOCKER_IMG_NAME "$@"
fi
