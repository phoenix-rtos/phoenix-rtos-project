#!/bin/bash

DOCKER_IMG_NAME=phoenixrtos/build
DOCKER_USER="$(id -u):$(id -g)"

TMPFS_OVERLAY=()
if [ "$(uname)" = "Darwin" ]; then
    # I/O operations on bind mounts in Darwin are painfully slow - use tmpfs for intermediate build artifacts
    chmod 777 "_build"  # fix against invalid tmpfs permissions
    TMPFS_OVERLAY=("--tmpfs"  "/src/_build:exec")
fi

if [ "$#" -eq 1 ] && [ "$1" = "bash" ]; then
    # run interactive shell - using ROOT user
    exec docker run -it  --rm -v "$(pwd):/src" -w /src -e TARGET -e SYSPAGE -e CONSOLE --entrypoint bash $DOCKER_IMG_NAME
else
    # run build - use our own UID/GID to create files with correct owner
    exec docker run -it --user "$DOCKER_USER" --rm -v "$(pwd):/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" -e TARGET -e SYSPAGE -e CONSOLE $DOCKER_IMG_NAME "$@"
fi

