#!/bin/bash

DOCKER_IMG_NAME=docker://phoenixrtosbadochov/build:gcc-14.2
if [ -e .docker_build_img ]; then
    DOCKER_IMG_NAME="$(cat  .docker_build_img)"
fi
PATH_TO_PROJECT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")/"
DOCKER_USER="$(id -u):$(id -g)"

TMPFS_OVERLAY=()
if [ "$(uname)" = "Darwin" ]; then
    # I/O operations on bind mounts in Darwin are painfully slow - use tmpfs for intermediate build artifacts
    chmod 777 "_build"  # fix against invalid tmpfs permissions
    TMPFS_OVERLAY=("--tmpfs"  "/src/_build:exec")
fi

if [ "$#" -eq 1 ] && [ "$1" = "bash" ]; then
    # run interactive shell - using ROOT user
    exec docker run -it --rm -v "${PATH_TO_PROJECT}:/src" -w /src -e TARGET -e SYSPAGE -e CONSOLE -e LONG_TEST -e SIL -e DEBUG --entrypoint bash "$DOCKER_IMG_NAME"
else
    # run build - use our own UID/GID to create files with correct owner
    exec docker run -it --user "$DOCKER_USER" --rm -v "${PATH_TO_PROJECT}:/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" -e TARGET -e SYSPAGE -e CONSOLE -e LONG_TEST -e SIL -e DEBUG "$DOCKER_IMG_NAME" "$@"
fi

