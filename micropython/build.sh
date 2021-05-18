#!/bin/bash

set -e

UPYTH_VER="1.15"
UPYTH="micropython-${UPYTH_VER}"

b_log "Building micropython"
PREFIX_UPYTH="${TOPDIR}/phoenix-rtos-ports/micropython"
PREFIX_UPYTH_BUILD="${PREFIX_BUILD}/micropython"
PREFIX_UPYTH_SRC=${PREFIX_UPYTH_BUILD}/${UPYTH}
PREFIX_UPYTH_PATCHSET="${PREFIX_UPYTH}/${UPYTH}-patchset"
PREFIX_UPYTH_MARKERS="$PREFIX_UPYTH_BUILD/markers/"

#
# Download and unpack
#
mkdir -p "$PREFIX_UPYTH_BUILD" "$PREFIX_UPYTH_MARKERS"
[ -f "$PREFIX_UPYTH/${UPYTH}.tar.xz" ] || wget https://github.com/micropython/micropython/releases/download/v${UPYTH_VER}/${UPYTH}.tar.xz -P "$PREFIX_UPYTH"
[ -d "${PREFIX_UPYTH_SRC}" ] || tar xf "$PREFIX_UPYTH/${UPYTH}.tar.xz" -C "$PREFIX_UPYTH_BUILD"


#
# Apply patches
#
# FIXME: prefix patch files with numbers to ensure correct order
for patchfile in "$PREFIX_UPYTH_PATCHSET"/*.patch; do
	if [ ! -f "$PREFIX_UPYTH_MARKERS/$(basename "$patchfile").applied" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_UPYTH_SRC" -p1 < "$patchfile"   # FIXME: should be -p1
		touch "$PREFIX_UPYTH_MARKERS/$(basename "$patchfile").applied"
	fi
done

#
# Build and install micropython binary
#
# FIXME - remove clean
(cd "${PREFIX_UPYTH_SRC}/mpy-cross" && make clean && make all BUILD="${PREFIX_UPYTH_BUILD}")
(cd "${PREFIX_UPYTH_SRC}/ports/unix" && make clean && make all)

cp -a "${PREFIX_UPYTH_SRC}/ports/unix/micropython" "$PREFIX_PROG"
"${CROSS}strip" -s "$PREFIX_PROG/micropython" -o "$PREFIX_PROG_STRIPPED/micropython"
b_install "$PREFIX_PORTS_INSTALL/curl" /bin/
