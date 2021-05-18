#!/bin/bash

set -e

PCRE=pcre-8.42

b_log "Building pcre"
PREFIX_PCRE="${TOPDIR}/phoenix-rtos-ports/pcre"
PREFIX_PCRE_BUILD="${PREFIX_BUILD}/pcre"
PREFIX_PCRE_SRC="${PREFIX_PCRE_BUILD}/${PCRE}"

if [ -n "$CLEAN" ]; then
	rm -fr "${PREFIX_PCRE_BUILD:?}"/*
fi

#
# Download and unpack
#
mkdir -p "$PREFIX_PCRE_BUILD"
[ -f "$PREFIX_PCRE/${PCRE}.tar.bz2" ] || wget http://ftp.pcre.org/pub/pcre/${PCRE}.tar.bz2 -P "$PREFIX_PCRE"
[ -d "$PREFIX_PCRE_SRC" ] || tar jxf "$PREFIX_PCRE/${PCRE}.tar.bz2" -C "$PREFIX_PCRE_BUILD"


#
# Configure
#
if [ ! -f "${PREFIX_PCRE_BUILD}/config.h" ]; then
	( cd "${PREFIX_PCRE_BUILD}" && "${PREFIX_PCRE_SRC}/configure" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" ARFLAGS="-r" --enable-static --disable-shared --host="$HOST" \
		--disable-cpp --prefix="${PREFIX_PCRE_BUILD}" --libdir="${PREFIX_A}"  \
		--includedir="${PREFIX_H}" )
fi

#
# Make
#
make -C "$PREFIX_PCRE_BUILD" install
