#!/bin/bash

set -e

LIGHTTPD="lighttpd-1.4.53"

b_log "Building lighttpd"


PREFIX_LIGHTTPD=${TOPDIR}/phoenix-rtos-ports/lighttpd
PREFIX_LIGHTTPD_BUILD=${PREFIX_BUILD}/lighttpd
PREFIX_LIGHTTPD_SRC=${PREFIX_LIGHTTPD}/${LIGHTTPD}

PREFIX_OPENSSL=${PREFIX_BUILD}
PREFIX_PCRE=${PREFIX_BUILD}

#
# Download and unpack
#
mkdir -p "$PREFIX_LIGHTTPD_BUILD"

if [ ! -z "$CLEAN" ]; then
	[ -f "$PREFIX_LIGHTTPD/${LIGHTTPD}.tar.gz" ] || wget https://download.lighttpd.net/lighttpd/releases-1.4.x/${LIGHTTPD}.tar.gz -P "$PREFIX_LIGHTTPD" --no-check-certificate
	if [ ! -d "$PREFIX_LIGHTTPD_SRC" ];then
		tar zxf "$PREFIX_LIGHTTPD/${LIGHTTPD}.tar.gz" -C "$PREFIX_LIGHTTPD"

		for patchfile in ${PREFIX_LIGHTTPD}/patches/*.patch; do
			echo "applying patch: $patchfile"
			patch -d $PREFIX_LIGHTTPD_SRC -p1 -p1 < $patchfile 
		done
	fi


	CONFIGFILE=$(find "${TOPDIR}/_fs/root-skel/etc" -name "lighttpd.conf")
	grep mod_ $CONFIGFILE | cut -d'"' -f2 | xargs -L1 -I{} echo "PLUGIN_INIT({})" > $PREFIX_LIGHTTPD_SRC/src/plugin-static.h

	[ -f "$PREFIX_LIGHTTPD_SRC/config.cache" ] && rm $PREFIX_LIGHTTPD_SRC/config.cache
	LIGHTTPD_CFLAGS="-DLIGHTTPD_STATIC -DPHOENIX"

#
# Configure
#

	( cd $PREFIX_LIGHTTPD_BUILD && $PREFIX_LIGHTTPD_SRC/configure LIGHTTPD_STATIC=yes CFLAGS="${LIGHTTPD_CFLAGS} ${CFLAGS}" CPPFLAGS="" LDFLAGS="${LDFLAGS}" AR_FLAGS="-r" \
		-C --disable-ipv6 --disable-mmap --with-bzip2=no \
		--with-zlib=no --enable-shared=no --enable-static=yes --disable-shared  --host="$HOST" --with-openssl=${PREFIX_OPENSSL} --with-pcre=${PREFIX_PCRE} \
		--prefix="$PREFIX_LIGHTTPD_BUILD" --sbindir="$PREFIX_PROG")

	set +e
	ex "+/HAVE_MMAP 1/d" "+/HAVE_MUNMAP 1/d" "+/HAVE_GETRLIMIT 1/d" "+/HAVE_SYS_POLL_H 1/d" \
	   "+/HAVE_SIGACTION 1/d" "+/HAVE_DLFCN_H 1/d" -cwq $PREFIX_LIGHTTPD_BUILD/config.h
	set -e
fi

#
# Make
#

make -C ${PREFIX_LIGHTTPD_BUILD} -f ${PREFIX_LIGHTTPD_BUILD}/Makefile  CROSS_COMPILE="$CROSS"  ${MAKEFLAGS} install

mkdir -p $PREFIX_PROG_STRIPPED && ${CROSS}strip -s $PREFIX_PROG/lighttpd -o $PREFIX_PROG_STRIPPED/lighttpd
b_install "$PREFIX_PORTS_INSTALL/lighttpd" /sbin

