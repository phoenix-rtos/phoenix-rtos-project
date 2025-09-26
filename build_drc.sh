#!/bin/bash
#
# Shell script for building COGNIT Device-Runtime-C
#
# Copyright 2025 Phoenix Systems
# Author: Mateusz Kobak
#
set -e

unset DEBUG

CFLAGS="$EXPORT_CFLAGS"
LDFLAGS="$EXPORT_LDFLAGS"
STRIP="$EXPORT_STRIP"
export CFLAGS LDFLAGS STRIP

PREFIX_BUILD_DRC="${PREFIX_BUILD}/device-runtime-c"

cmake -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY -DCOGNIT_BUILD_EXAMPLES=OFF -DCOGNIT_LOG_LEVEL=3 -S "./device-runtime-c" -B "${PREFIX_BUILD_DRC}"
make -C "${PREFIX_BUILD_DRC}" -j 9
cp "${PREFIX_BUILD_DRC}/cognit/"*".a" "${PREFIX_A}"
cp "./device-runtime-c/cognit/nanopb/"*".h" "${PREFIX_H}"
mkdir -p "${PREFIX_H}/cognit/"
cp "./device-runtime-c/cognit/include/"*".h" "${PREFIX_H}/cognit/"
