#!/bin/bash

set -e

DROPBEAR=dropbear-2018.76

b_log "Building dropbear"
PREFIX_DROPBEAR="${TOPDIR}/phoenix-rtos-ports/dropbear"
PREFIX_DROPBEAR_BUILD="${PREFIX_BUILD}/dropbear"
PREFIX_DROPBEAR_SRC="${PREFIX_DROPBEAR_BUILD}/${DROPBEAR}"
PREFIX_DROPBEAR_MARKERS="${PREFIX_DROPBEAR_BUILD}/markers"

#
# Download and unpack
#
mkdir -p "$PREFIX_DROPBEAR_BUILD" "$PREFIX_DROPBEAR_MARKERS"
[ -f "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" ] || wget http://matt.ucc.asn.au/dropbear/releases/${DROPBEAR}.tar.bz2 -P "${PREFIX_DROPBEAR}" --no-check-certificate
[ -d "$PREFIX_DROPBEAR_SRC" ] || ( tar jxf "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" -C "${PREFIX_DROPBEAR_BUILD}" && rm -rf "${PREFIX_DROPBEAR_MARKERS:?}/*" )

#
# Apply patches
#
for patchfile in "$PREFIX_DROPBEAR"/patch/*; do
	if [ ! -f "$PREFIX_DROPBEAR_MARKERS/$(basename "$patchfile").applied" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_DROPBEAR_SRC" -p1 < "$patchfile"
		touch "$PREFIX_DROPBEAR_MARKERS/$(basename "$patchfile").applied"
	fi
done

#
# Configure
#
if [ ! -f "$PREFIX_DROPBEAR_BUILD/config.h" ]; then
	cp -a "$PREFIX_DROPBEAR/localoptions.h" "$PREFIX_DROPBEAR_BUILD"

	DROPBEAR_CFLAGS="-DENDIAN_LITTLE -DUSE_DEV_PTMX ${DROPBEAR_CUSTOM_CFLAGS}"
	DROPBEAR_LDFLAGS=""

	( cd "${PREFIX_DROPBEAR_BUILD}" && "${PREFIX_DROPBEAR_SRC}/configure" CPPFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" CFLAGS="${CFLAGS} ${DROPBEAR_CFLAGS}" \
		LDFLAGS="${CFLAGS} ${LDFLAGS} ${DROPBEAR_LDFLAGS}" ARFLAGS="-r" \
		--host="$HOST_TARGET" --includedir="${PREFIX_H}"  \
		--prefix="${PREFIX_PROG}" --program-prefix="${PREFIX_PROG}" --libdir="${PREFIX_A}" --bindir="${PREFIX_PROG}" --disable-zlib --enable-static \
		--disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-harden )
fi

#
# Make
#
make PROGRAMS="dropbear dbclient dropbearkey scp" -C "${PREFIX_DROPBEAR_BUILD}" CROSS_COMPILE="$CROSS"

"${CROSS}strip" -s "$PREFIX_DROPBEAR_BUILD/dropbear" -o "$PREFIX_PROG_STRIPPED/dropbear"
"${CROSS}strip" -s "$PREFIX_DROPBEAR_BUILD/dbclient" -o "$PREFIX_PROG_STRIPPED/dbclient"
"${CROSS}strip" -s "$PREFIX_DROPBEAR_BUILD/scp" -o "$PREFIX_PROG_STRIPPED/scp"
cp -a "$PREFIX_DROPBEAR_BUILD/dropbear" "$PREFIX_PROG/dropbear"
cp -a "$PREFIX_DROPBEAR_BUILD/dbclient" "$PREFIX_PROG/dbclient"
cp -a "$PREFIX_DROPBEAR_BUILD/scp" "$PREFIX_PROG/scp"

b_install "$PREFIX_PORTS_INSTALL/dropbear" /sbin
b_install "$PREFIX_PORTS_INSTALL/dbclient" /usr/bin
b_install "$PREFIX_PORTS_INSTALL/scp" /bin
