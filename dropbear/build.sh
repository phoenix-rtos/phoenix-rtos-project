#!/bin/bash
DROPBEAR=dropbear-2018.76

b_log "Building dropbear"
PREFIX_DROPBEAR=${TOPDIR}/phoenix-rtos-ports/dropbear
PREFIX_DROPBEAR_BUILD=${PREFIX_BUILD}/dropbear
PREFIX_DROPBEAR_SRC=${PREFIX_DROPBEAR}/${DROPBEAR}

#
# Download and unpack
#


if [ ! -z "$CLEAN" ]; then

	rm -fr $PREFIX_DROPBEAR_BUILD
	mkdir -p "$PREFIX_DROPBEAR_BUILD"
	
	[ -f "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" ] || wget http://matt.ucc.asn.au/dropbear/releases/${DROPBEAR}.tar.bz2 -P "${PREFIX_DROPBEAR}"
	[ -d "$PREFIX_DROPBEAR_SRC" ] || tar jxf "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" -C "${PREFIX_DROPBEAR}"

	cp $PREFIX_DROPBEAR/localoptions.h $PREFIX_DROPBEAR_BUILD

	DROPBEAR_CFLAGS=""
	DROPBEAR_LDFLAGS=""

#
# Configure
#
	( cd ${PREFIX_DROPBEAR_BUILD} && ${PREFIX_DROPBEAR_SRC}/configure CPPFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" CFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" \
		LDFLAGS="${CFLAGS} ${LDFLAGS} ${DROPBEAR_LDFLAGS}" ARFLAGS="-r" \
		--host="$TARGET_FAMILY" --target="$TARGET_FAMILY" CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as RANLIB=${CROSS}gcc-ranlib \
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

b_install "$PREFIX_PROG_STRIPPED/dropbear" /sbin
b_install "$PREFIX_PROG_STRIPPED/dbclient" /usr/bin
b_install "$PREFIX_PROG_STRIPPED/scp" /bin

exit 0
