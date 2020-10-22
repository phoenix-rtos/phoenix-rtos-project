#!/bin/sh

DOCKER_IMG_NAME=phoenixrtos/build
DOCKER_USER="$(id -u):$(id -g)"

if [ "$#" -eq 1 ] && [ "$1" = "bash" ]; then
    # run interactive shell - using ROOT user
    exec docker run -it  --rm -v "$(pwd):/src" -w /src -e TARGET --entrypoint bash $DOCKER_IMG_NAME
else
    # run build - use our own UID/GID to create files with correct owner
    exec docker run -it --user "$DOCKER_USER" --rm -v "$(pwd):/src:delegated" -w /src -e TARGET $DOCKER_IMG_NAME "$@"
fi

