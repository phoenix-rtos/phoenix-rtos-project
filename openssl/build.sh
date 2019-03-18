#!/bin/bash
OPENSSL=openssl-1.1.1a

b_log "Building openssl"
PREFIX_OPENSSL=${TOPDIR}/phoenix-rtos-ports/openssl
PREFIX_OPENSSL_BUILD=${PREFIX_BUILD}/openssl
PREFIX_OPENSSL_SRC=${PREFIX_OPENSSL}/${OPENSSL}

#
# Download and unpack
#
mkdir -p "$PREFIX_OPENSSL_BUILD"

if [ ! -z "$CLEAN" ]; then

	rm -fr $PREFIX_OPENSSL_BUILD/*

	[ -f "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" ] || wget https://www.openssl.org/source/${OPENSSL}.tar.gz -P "$PREFIX_OPENSSL"
	[ -d "$PREFIX_OPENSSL_SRC" ] || tar zxf "$PREFIX_OPENSSL/${OPENSSL}.tar.gz" -C "$PREFIX_OPENSSL"

	OPENSSL_CFLAGS=""
	OPENSSL_LDFLAGS=""
#
# Configure
#
	cp $PREFIX_OPENSSL/30-phoenix.conf $PREFIX_OPENSSL_SRC/Configurations/
	(cd ${PREFIX_OPENSSL_BUILD} && ${PREFIX_OPENSSL_SRC}/Configure phoenix-arm --prefix=$PREFIX_OPENSSL_BUILD)
fi

#
# Make
#
make -C "$PREFIX_OPENSSL_SRC" ${MAKEFLAGS} all
make -C "$PREFIX_OPENSSL_SRC" ${MAKEFLAGS} install_sw
