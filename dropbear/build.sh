#!/bin/bash

set -e

DROPBEAR=dropbear-2018.76

b_log "Building dropbear"
PREFIX_DROPBEAR=${TOPDIR}/phoenix-rtos-ports/dropbear
PREFIX_DROPBEAR_BUILD=${PREFIX_BUILD}/dropbear
PREFIX_DROPBEAR_SRC=${PREFIX_DROPBEAR}/${DROPBEAR}
PREFIX_DROPBEAR_MARKERS=${PREFIX_DROPBEAR_BUILD}/markers
#
# Download and unpack
#


if [ ! -z "$CLEAN" ]; then

	mkdir -p "$PREFIX_DROPBEAR_BUILD"
	mkdir -p "$PREFIX_DROPBEAR_MARKERS"
	
	[ -f "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" ] || wget http://matt.ucc.asn.au/dropbear/releases/${DROPBEAR}.tar.bz2 -P "${PREFIX_DROPBEAR}"
	[ -d "$PREFIX_DROPBEAR_SRC" ] || ( tar jxf "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" -C "${PREFIX_DROPBEAR}" && find -P "$PREFIX_DROPBEAR_MARKERS" -size 0 -type f -name "*.applied" -delete )

	cp $PREFIX_DROPBEAR/localoptions.h $PREFIX_DROPBEAR_BUILD

	DROPBEAR_CFLAGS="-DENDIAN_LITTLE -DUSE_DEV_PTMX ${DROPBEAR_CUSTOM_CFLAGS}"
	DROPBEAR_LDFLAGS=""
fi

for patchfile in $PREFIX_DROPBEAR/patch/*; do
	if [ ! -f "$PREFIX_DROPBEAR_MARKERS/$(basename $patchfile.applied)" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_DROPBEAR_SRC" -p1 < "$patchfile"
		touch "$PREFIX_DROPBEAR_MARKERS/$(basename $patchfile).applied"
	fi
done

if [ ! -z "$CLEAN" ]; then
#
# Configure
#
	( cd ${PREFIX_DROPBEAR_BUILD} && ${PREFIX_DROPBEAR_SRC}/configure CPPFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" CFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" \
		LDFLAGS="${CFLAGS} ${LDFLAGS} ${DROPBEAR_LDFLAGS}" ARFLAGS="-r" \
		--host="$HOST_TARGET" CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as RANLIB=${CROSS}gcc-ranlib \
		--includedir="${PREFIX_PROG}" --oldincludedir="${PREFIX_PROG}" \
		--prefix="${PREFIX_PROG}" --program-prefix="${PREFIX_PROG}" --libdir="${PREFIX_PROG}" --bindir="${PREFIX_PROG}" --disable-zlib --enable-static \
		--disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-harden )

fi
#
# Make
#
make PROGRAMS="dropbear dbclient dropbearkey scp" -C ${PREFIX_DROPBEAR_BUILD} -f ${PREFIX_DROPBEAR_BUILD}/Makefile CROSS_COMPILE="$CROSS" ${MAKEFLAGS} 

${CROSS}strip -s $PREFIX_DROPBEAR_BUILD/dropbear -o $PREFIX_PROG_STRIPPED/dropbear
${CROSS}strip -s $PREFIX_DROPBEAR_BUILD/dbclient -o $PREFIX_PROG_STRIPPED/dbclient
${CROSS}strip -s $PREFIX_DROPBEAR_BUILD/scp -o $PREFIX_PROG_STRIPPED/scp
cp -a $PREFIX_DROPBEAR_BUILD/dropbear $PREFIX_PROG/dropbear
cp -a $PREFIX_DROPBEAR_BUILD/dbclient $PREFIX_PROG/dbclient
cp -a $PREFIX_DROPBEAR_BUILD/scp $PREFIX_PROG/scp

b_install "$PREFIX_PORTS_INSTALL/dropbear" /sbin
b_install "$PREFIX_PORTS_INSTALL/dbclient" /usr/bin
b_install "$PREFIX_PORTS_INSTALL/scp" /bin

exit 0
