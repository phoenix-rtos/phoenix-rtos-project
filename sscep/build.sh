#!/bin/bash

set -e

SSCEP=sscep-0.9.0

b_log "Building sscep"
PREFIX_SSCEP=${TOPDIR}/phoenix-rtos-ports/sscep
PREFIX_SSCEP_BUILD=${PREFIX_BUILD}/sscep
PREFIX_SSCEP_SRC="${PREFIX_SSCEP_BUILD}/${SSCEP}"
PREFIX_SSCEP_MARKERS="${PREFIX_SSCEP_BUILD}/markers"
PREFIX_SSCEP_INSTALL="${PREFIX_SSCEP_BUILD}/install"

#
# Download and unpack
#
mkdir -p "$PREFIX_SSCEP_BUILD"

[ -f "${PREFIX_SSCEP}/${SSCEP}.tar.gz" ] || wget https://github.com/certnanny/sscep/archive/refs/tags/v0.9.0.tar.gz -O "${PREFIX_SSCEP}/${SSCEP}.tar.gz" --no-check-certificate
[ -d "$PREFIX_SSCEP_SRC" ] || tar zxf "$PREFIX_SSCEP/${SSCEP}.tar.gz" -C "$PREFIX_SSCEP_BUILD"

#
# Apply patches
#
mkdir -p "$PREFIX_SSCEP_MARKERS"

for patchfile in "${PREFIX_SSCEP}"/patches/*.patch; do
	if [ ! -f "${PREFIX_SSCEP_MARKERS}/$(basename "$patchfile").applied" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_SSCEP_SRC" -p1 < "$patchfile"
		touch "${PREFIX_SSCEP_MARKERS}/$(basename "$patchfile").applied"
	fi
done

#
# Make
#
mkdir -p "$PREFIX_SSCEP_INSTALL"

pushd "$PREFIX_SSCEP_SRC"
[ -f "${PREFIX_SSCEP_SRC}/configure" ] || ./bootstrap.sh
[ -f "${PREFIX_SSCEP_SRC}/Makefile" ] || ./configure --disable-shared --prefix="" --host="$HOST" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" PKG_CONFIG_LIBDIR="${PREFIX_BUILD}/lib/pkgconfig"
make install DESTDIR="$PREFIX_SSCEP_INSTALL"
popd

cp -a "${PREFIX_SSCEP_INSTALL}/bin/sscep" "${PREFIX_PROG}/sscep"
"${CROSS}strip" -s "${PREFIX_PROG}/sscep" -o "${PREFIX_PROG_STRIPPED}/sscep"
b_install "${PREFIX_PORTS_INSTALL}/sscep" /usr/bin
