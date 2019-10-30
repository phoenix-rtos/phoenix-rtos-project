#!/bin/bash

set -e

JANSSON=jansson-2.12

b_log "Building jansson"
PREFIX_JANSSON=${TOPDIR}/phoenix-rtos-ports/jansson
PREFIX_JANSSON_BUILD=${PREFIX_BUILD}/jansson
PREFIX_JANSSON_SRC=${PREFIX_JANSSON}/${JANSSON}

#
# Download and unpack
#
mkdir -p "$PREFIX_JANSSON_BUILD"

if [ ! -z "$CLEAN" ]; then

	rm -fr "${PREFIX_JANSSON_BUILD:?}/*"

	[ -f "$PREFIX_JANSSON/${JANSSON}.tar.bz2" ] || wget http://www.digip.org/jansson/releases/${JANSSON}.tar.bz2 -P "$PREFIX_JANSSON"
	[ -d "$PREFIX_JANSSON_SRC" ] || tar jxf "$PREFIX_JANSSON/${JANSSON}.tar.bz2" -C "$PREFIX_JANSSON"

#
# Configure
#
	( cd "${PREFIX_JANSSON_BUILD}" && "${PREFIX_JANSSON_SRC}/configure" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" ARFLAGS="-r" --enable-static --disable-shared --host="$HOST_TARGET" \
                --disable-cpp CC="${CROSS}gcc" AR="${CROSS}ar" LD="${CROSS}ld" AS="${CROSS}as" RANLIB="${CROSS}gcc-ranlib" --prefix="${PREFIX_JANSSON_BUILD}" --libdir="${PREFIX_BUILD}/lib" \
                --includedir="${PREFIX_BUILD}/include" )

make -C "$PREFIX_JANSSON_BUILD" ${MAKEFLAGS} ${CLEAN}

fi

#
# Make
#
make -C "$PREFIX_JANSSON_BUILD" ${MAKEFLAGS} install
