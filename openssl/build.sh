#!/bin/bash

set -e

OPENSSL=openssl-1.1.1a

b_log "Building openssl"
PREFIX_OPENSSL=${TOPDIR}/phoenix-rtos-ports/openssl
PREFIX_OPENSSL_BUILD=${PREFIX_BUILD}/openssl
PREFIX_OPENSSL_SRC=${PREFIX_OPENSSL}/${OPENSSL}
PREFIX_OPENSSL_INSTALL=$PREFIX_OPENSSL_BUILD/install

#
# Download and unpack
#
mkdir -p "$PREFIX_OPENSSL_INSTALL"

pushd $PREFIX_OPENSSL_BUILD
if [ ! -z "$CLEAN" ]; then

	rm -fr $PREFIX_OPENSSL_BUILD/*

	[ -f "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" ] || wget https://www.openssl.org/source/${OPENSSL}.tar.gz -P "$PREFIX_OPENSSL" --no-check-certificate
	[ -d "$PREFIX_OPENSSL_SRC" ] || tar zxf "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" -C "$PREFIX_OPENSSL"

#
# Configure
#
	cp $PREFIX_OPENSSL/30-phoenix.conf $PREFIX_OPENSSL_SRC/Configurations/
	
	${PREFIX_OPENSSL_SRC}/Configure phoenix-${TARGET_FAMILY}-${TARGET_SUBFAMILY} --prefix=$PREFIX_OPENSSL_INSTALL
	
fi

#
# Make
#
make -C "$PREFIX_OPENSSL_BUILD" ${MAKEFLAGS} all
make -C "$PREFIX_OPENSSL_BUILD" ${MAKEFLAGS} install_sw
popd

mkdir -p $PREFIX_BUILD/include && cp -Ra $PREFIX_OPENSSL_INSTALL/include/openssl $PREFIX_BUILD/include
mkdir -p $PREFIX_BUILD/lib && cp -Ra $PREFIX_OPENSSL_INSTALL/lib/libcrypto.a $PREFIX_BUILD/lib/
cp -Ra $PREFIX_OPENSSL_INSTALL/lib/libssl.a  $PREFIX_BUILD/lib/

mkdir -p $PREFIX_PROG && cp -Ra $PREFIX_OPENSSL_INSTALL/bin/openssl  $PREFIX_PROG/openssl
mkdir -p $PREFIX_PROG_STRIPPED && ${CROSS}strip -s $PREFIX_PROG/openssl -o $PREFIX_PROG_STRIPPED/openssl
b_install "$PREFIX_PORTS_INSTALL/openssl" /usr/bin/
