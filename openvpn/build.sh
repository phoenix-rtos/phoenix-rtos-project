#!/bin/bash

set -e

OPENVPN=openvpn-2.4.7

b_log "Building openvpn"
PREFIX_OPENVPN=${TOPDIR}/phoenix-rtos-ports/openvpn
PREFIX_OPENVPN_BUILD=${PREFIX_BUILD}/openvpn
PREFIX_OPENVPN_SRC=${PREFIX_OPENVPN}/${OPENVPN}

#
# Download and unpack
#
mkdir -p "$PREFIX_OPENVPN_BUILD"

if [ ! -z "$CLEAN" ]; then

	rm -fr $PREFIX_OPENVPN_BUILD/*

	[ -f "$PREFIX_OPENVPN/${OPENVPN}.tar.gz" ] || wget https://swupdate.openvpn.org/community/releases/${OPENVPN}.tar.gz -P "$PREFIX_OPENVPN" --no-check-certificate
	if [ ! -d "$PREFIX_OPENVPN_SRC" ]; then
	   	tar zxf "$PREFIX_OPENVPN/${OPENVPN}.tar.gz" -C "$PREFIX_OPENVPN"

#
#  Apply patch
#
		cd $PREFIX_OPENVPN_SRC
		for i in ${PREFIX_OPENVPN}/*.patch; do patch -t -p1 < $i; done

	fi
	OPENVPN_CFLAGS="-std=gnu99 -I${PREFIX_BUILD}/include"
#
# Configure
#
	cd $PREFIX_OPENVPN_SRC
	autoreconf -i -v -f
	cd $PREFIX_OPENVPN_BUILD && PKG_CONFIG="" $PREFIX_OPENVPN_SRC/configure CFLAGS="$CFLAGS $OPENVPN_CFLAGS" LDFLAGS="$LDFLAGS" --host="${HOST}" --sbindir=$PREFIX_PROG
fi

#
# Make
#
make -C "$PREFIX_OPENVPN_BUILD" $MAKEFLAGS
make -C "$PREFIX_OPENVPN_BUILD" install-exec

mkdir -p $PREFIX_PROG_STRIPPED && ${CROSS}strip -s $PREFIX_PROG/openvpn -o $PREFIX_PROG_STRIPPED/openvpn
b_install "$PREFIX_PORTS_INSTALL/openvpn" /sbin/
