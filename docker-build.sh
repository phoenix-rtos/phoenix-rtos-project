#!/bin/bash

DOCKER_IMG_NAME=phoenixrtos/build
if [ -e .docker_build_img ]; then
    DOCKER_IMG_NAME="$(cat .docker_build_img)"
fi
PATH_TO_PROJECT="$(dirname "$(realpath "${BASH_SOURCE[0]}")")/"
DOCKER_USER="$(id -u):$(id -g)"

RUST_ARGS=()
if [ "$RUST_ENABLE" = "y" ] && [ "$*" != "pull" ]; then
    if [ -n "$RUST_TOOLCHAIN_MOUNT" ]; then
        RUST_TOOLCHAIN_HOST="$(realpath "$RUST_TOOLCHAIN_MOUNT")"
    else
        RUST_TOOLCHAIN_HOST="$("${PATH_TO_PROJECT}scripts/rust-toolchain.sh")" || exit 1
    fi
    RUST_ARGS=(-v "${RUST_TOOLCHAIN_HOST}:/opt/phoenix/rust:ro" -e RUST_TOOLCHAIN_DIR=/opt/phoenix/rust -e CARGO_HOME=/src/_build/.cargo-home)
fi

TMPFS_OVERLAY=()
if [ "$(uname)" = "Darwin" ]; then
    # I/O operations on bind mounts in Darwin are painfully slow - use tmpfs for intermediate build artifacts
    chmod 777 "_build"  # fix against invalid tmpfs permissions
    TMPFS_OVERLAY=("--tmpfs"  "/src/_build:exec,size=2g")
fi

if [ "$#" -eq 1 ] && [ "$1" = "bash" ]; then
    # run interactive shell - using ROOT user
    exec docker run -it --rm -h "$(hostname)-docker" --env-file .docker_env "${RUST_ARGS[@]}" -v "${PATH_TO_PROJECT}:/src" -w /src --entrypoint bash "$DOCKER_IMG_NAME"
elif [ "$#" -eq 1 ] && [ "$1" = "pull" ]; then
    # explicitly pull (update local image)
    exec docker pull "$DOCKER_IMG_NAME"
else
    # run build - use our own UID/GID to create files with correct owner
    exec docker run -it --rm -h "$(hostname)-docker" --env-file .docker_env "${RUST_ARGS[@]}" --user "$DOCKER_USER" -v "${PATH_TO_PROJECT}:/src:delegated" -w /src "${TMPFS_OVERLAY[@]}" "$DOCKER_IMG_NAME" "$@"
fi

