#!/bin/bash

set -e

OPENSSL=openssl-1.1.1a

b_log "Building openssl"
PREFIX_OPENSSL="${TOPDIR}/phoenix-rtos-ports/openssl"
PREFIX_OPENSSL_BUILD="${PREFIX_BUILD}/openssl"
PREFIX_OPENSSL_SRC="${PREFIX_OPENSSL_BUILD}/${OPENSSL}"
PREFIX_OPENSSL_INSTALL="$PREFIX_OPENSSL_BUILD/install"

#
# Download and unpack
#
mkdir -p "$PREFIX_OPENSSL_BUILD" "$PREFIX_OPENSSL_INSTALL"
[ -f "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" ] || wget https://www.openssl.org/source/${OPENSSL}.tar.gz -P "$PREFIX_OPENSSL" --no-check-certificate
[ -d "$PREFIX_OPENSSL_SRC" ] || tar zxf "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" -C "$PREFIX_OPENSSL_BUILD"

#
# Configure
#
if [ ! -f "${PREFIX_OPENSSL_BUILD}/Makefile" ]; then
	cp "$PREFIX_OPENSSL/30-phoenix.conf" "$PREFIX_OPENSSL_SRC/Configurations/"
	(cd "${PREFIX_OPENSSL_BUILD}" && "${PREFIX_OPENSSL_SRC}/Configure" "phoenix-${TARGET_FAMILY}-${TARGET_SUBFAMILY}" --prefix="$PREFIX_OPENSSL_INSTALL")
fi


#
# Make
#
make -C "$PREFIX_OPENSSL_BUILD" all
make -C "$PREFIX_OPENSSL_BUILD" install_sw

cp -a "$PREFIX_OPENSSL_INSTALL/include/openssl" "$PREFIX_H"
cp -a "$PREFIX_OPENSSL_INSTALL/lib/libcrypto.a" "$PREFIX_A"
cp -a "$PREFIX_OPENSSL_INSTALL/lib/libssl.a"  "$PREFIX_A"

cp -a "$PREFIX_OPENSSL_INSTALL/bin/openssl"  "$PREFIX_PROG"
"${CROSS}strip" -s "$PREFIX_PROG/openssl" -o "$PREFIX_PROG_STRIPPED/openssl"
b_install "$PREFIX_PORTS_INSTALL/openssl" /usr/bin/
