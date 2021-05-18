#!/bin/bash

set -e

CURL=curl-7.64.1

b_log "Building curl"
PREFIX_CURL="${TOPDIR}/phoenix-rtos-ports/curl"
PREFIX_CURL_BUILD="${PREFIX_BUILD}/curl"
PREFIX_CURL_SRC="${PREFIX_CURL_BUILD}/${CURL}"
PREFIX_CURL_INSTALL="$PREFIX_CURL_BUILD/install"

#
# Download and unpack
#
mkdir -p "$PREFIX_CURL_BUILD" "$PREFIX_CURL_INSTALL"
[ -f "$PREFIX_CURL/${CURL}.tar.gz" ] || wget https://curl.haxx.se/download/${CURL}.tar.gz -P "$PREFIX_CURL" --no-check-certificate
[ -d "$PREFIX_CURL_SRC" ] || tar zxf "$PREFIX_CURL/${CURL}.tar.gz" -C "$PREFIX_CURL_BUILD"


#
# Configure
#
if [ ! -f "$PREFIX_CURL_BUILD/config.status" ]; then
	( cd "$PREFIX_CURL_BUILD" && PKG_CONFIG="" "$PREFIX_CURL_SRC/configure" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" \
		--host="${HOST}" --sbindir="$PREFIX_PROG" --disable-pthreads --disable-threaded-resolver \
		--disable-ipv6 --prefix="$PREFIX_CURL_INSTALL" --disable-ntlm-wb --without-zlib )
fi

#
# Make
#
make -C "$PREFIX_CURL_BUILD"
make -C "$PREFIX_CURL_BUILD" install

cp -a "$PREFIX_CURL_INSTALL/include/curl" "$PREFIX_H"
cp -a "$PREFIX_CURL_INSTALL/lib/"* "$PREFIX_A"
cp -a "$PREFIX_CURL_INSTALL/bin/curl" "$PREFIX_PROG/curl"
"${CROSS}strip" -s "$PREFIX_PROG/curl" -o "$PREFIX_PROG_STRIPPED/curl"
b_install "$PREFIX_PORTS_INSTALL/curl" /bin/
