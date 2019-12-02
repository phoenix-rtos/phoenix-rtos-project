#!/bin/bash

set -e
BUSYBOX=busybox-1.27.2

b_log "Building busybox"
PREFIX_BUSYBOX=${TOPDIR}/phoenix-rtos-ports/busybox
PREFIX_BUSYBOX_BUILD=$PREFIX_BUILD/busybox/
PREFIX_BUSYBOX_SRC=$PREFIX_BUSYBOX/${BUSYBOX}/
PREFIX_BUSYBOX_MARKERS=$PREFIX_BUSYBOX_BUILD/markers/
: "${BUSYBOX_CONFIG:="${PREFIX_BUSYBOX}/config"}"

echo $PREFIX_BUSYBOX_SRC

#
# Download and unpack
#
mkdir -p "$PREFIX_BUSYBOX_BUILD" "$PREFIX_BUSYBOX_MARKERS"
[ -f "$PREFIX_BUSYBOX/${BUSYBOX}.tar.bz2" ] || wget "http://busybox.net/downloads/${BUSYBOX}.tar.bz2" -P "$PREFIX_BUSYBOX" --no-check-certificate
[ -d "$PREFIX_BUSYBOX_SRC" ] || ( tar jxf "$PREFIX_BUSYBOX/${BUSYBOX}.tar.bz2" -C "$PREFIX_BUSYBOX" && find -P "$PREFIX_BUSYBOX_MARKERS" -size 0 -type f -name "*.applied" -delete )

#
# Apply patches
#
for patchfile in $PREFIX_BUSYBOX/*.patch; do
	if [ ! -f "$PREFIX_BUSYBOX_MARKERS/$(basename $patchfile.applied)" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_BUSYBOX_SRC" -p1 < "$patchfile"
		touch "$PREFIX_BUSYBOX_MARKERS/$(basename $patchfile).applied"
	fi
done

#
# Clean and configure
#
if [ ! -z $CLEAN ] || [ ! -f "${PREFIX_BUSYBOX_BUILD}/.config" ] || [ "${BUSYBOX_CONFIG}" -nt "${PREFIX_BUSYBOX_BUILD}/.config" ]; then
	cp -a "${BUSYBOX_CONFIG}" "${PREFIX_BUSYBOX_BUILD}"/.config
	make -C ${PREFIX_BUSYBOX_BUILD} KBUILD_SRC="$PREFIX_BUSYBOX_SRC" -f "${PREFIX_BUSYBOX_SRC}"/Makefile CROSS_COMPILE="$CROSS" CONFIG_PREFIX="$PREFIX_FS/root" ${MAKEFLAGS} clean
fi

# hackish: remove the final binary to re-link potential libc changes
rm -rf "$PREFIX_BUSYBOX_BUILD/busybox_unstripped" "$PREFIX_BUSYBOX_BUILD/busybox"

# For MacOS
export LC_CTYPE=C
if [[ -n "$PORTS_INSTALL_STRIPPED" && "$PORTS_INSTALL_STRIPPED" = "n" ]]; then
	UNSTRIPPED=y
else
	UNSTRIPPED=n
fi

make -C ${PREFIX_BUSYBOX_BUILD} KBUILD_SRC="$PREFIX_BUSYBOX_SRC" -f "${PREFIX_BUSYBOX_SRC}"/Makefile CROSS_COMPILE="$CROSS" CONFIG_PREFIX="$PREFIX_FS/root" SKIP_STRIP="$UNSTRIPPED" all
make -C ${PREFIX_BUSYBOX_BUILD} KBUILD_SRC="$PREFIX_BUSYBOX_SRC" -f "${PREFIX_BUSYBOX_SRC}"/Makefile CROSS_COMPILE="$CROSS" CONFIG_PREFIX="$PREFIX_FS/root" SKIP_STRIP="$UNSTRIPPED" install
