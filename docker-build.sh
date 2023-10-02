#!/bin/bash

DOCKER_IMG_NAME=phoenixrtos/build
DOCKER_USER="$(id -u):$(id -g)"

TMPFS_OVERLAY=()
# FIXME: due to building as root in docker the tmpfs approach doesn't work
# if [ "$(uname)" = "Darwin" ]; then
#     # I/O operations on bind mounts in Darwin are painfully slow - use tmpfs for intermediate build artifacts
#     chmod 777 "_build"  # fix against invalid tmpfs permissions
#     TMPFS_OVERLAY=("--tmpfs"  "/src/_build:exec")
# fi

if [ "$#" -eq 1 ] && [ "$1" = "bash" ]; then
    # run interactive shell - using ROOT user
    exec docker run -it --rm -v "$(pwd):/src" -w /src -e TARGET -e SYSPAGE -e CONSOLE -e LONG_TEST --entrypoint bash "$DOCKER_IMG_NAME"
else
    # FIXME: run build - use our own UID/GID to create files with correct owner
    #exec docker run -it --user "$DOCKER_USER" --rm -v "$(pwd):/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" -e TARGET -e SYSPAGE -e CONSOLE $DOCKER_IMG_NAME "$@"

    # FOR NOW: run build as root to be able to overwrite files installed in toolchain
    docker run -it --rm -v "$(pwd):/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" -e TARGET -e SYSPAGE -e CONSOLE -e LONG_TEST "$DOCKER_IMG_NAME" "$@"
    BUILD_RET=$?

    # FIX file ownership in "_build"
    docker run -it --rm -v "$(pwd):/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" --entrypoint bash "$DOCKER_IMG_NAME" -c "chown -R \"$DOCKER_USER\" _build/ _fs/$TARGET _boot"
    OWNERSHIP_RET=$?

    if [ $BUILD_RET -ne 0 ]; then
        exit $BUILD_RET
    elif [ $OWNERSHIP_RET -ne 0 ]; then
        exit $OWNERSHIP_RET
    fi
fi

