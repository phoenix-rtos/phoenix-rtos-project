#!/bin/bash

set -e

CURL=curl-7.64.1

b_log "Building curl"
PREFIX_CURL=${TOPDIR}/phoenix-rtos-ports/curl
PREFIX_CURL_BUILD=${PREFIX_BUILD}/curl
PREFIX_CURL_SRC=${PREFIX_CURL}/${CURL}
PREFIX_CURL_INSTALL=$PREFIX_CURL_BUILD/install

#
# Download and unpack
#
mkdir -p $PREFIX_CURL_INSTALL

if [ ! -z "$CLEAN" ]; then

	rm -rf $PREFIX_CURL_BUILD/*

	[ -f "$PREFIX_CURL/${CURL}.tar.gz" ] || wget https://curl.haxx.se/download/${CURL}.tar.gz -P "$PREFIX_CURL"
	[ -d "$PREFIX_CURL_SRC" ] || tar zxf "$PREFIX_CURL/${CURL}.tar.gz" -C "$PREFIX_CURL"

#
# Configure
#
	cd $PREFIX_CURL_BUILD && PKG_CONFIG="" $PREFIX_CURL_SRC/configure CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" \
		--host="${HOST_TARGET}-phoenix" --sbindir=$PREFIX_PROG --disable-pthreads --disable-threaded-resolver \
		--disable-ipv6 --prefix=$PREFIX_CURL_INSTALL --disable-ntlm-wb
fi

#
# Make
#
make -C "$PREFIX_CURL_BUILD" $MAKEFLAGS
make -C "$PREFIX_CURL_BUILD" install

mkdir -p $PREFIX_BUILD/include && cp -Ra $PREFIX_CURL_INSTALL/include/curl $PREFIX_BUILD/include
mkdir -p $PREFIX_BUILD/lib && cp -Ra $PREFIX_CURL_INSTALL/lib/* $PREFIX_BUILD/lib/
mkdir -p $PREFIX_PROG && cp -Ra $PREFIX_CURL_INSTALL/bin/curl  $PREFIX_PROG/curl
mkdir -p $PREFIX_PROG_STRIPPED && ${CROSS}strip -s $PREFIX_PROG/curl -o $PREFIX_PROG_STRIPPED/curl
b_install "$PREFIX_PORTS_INSTALL/curl" /bin/
