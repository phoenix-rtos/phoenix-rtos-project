#!/bin/bash

set -e

UPYTH_VER="1.15"
UPYTH="micropython-${UPYTH_VER}"
DESTINATIONS_CFG="destinations.cfg"

PREFIX_UPYTH="${TOPDIR}/phoenix-rtos-ports/micropython"
PREFIX_UPYTH_BUILD="${PREFIX_BUILD}/micropython"
PREFIX_UPYTH_SRC=${PREFIX_UPYTH_BUILD}/${UPYTH}
PREFIX_UPYTH_CONFIG="${PREFIX_UPYTH}/${UPYTH}-config/"
PREFIX_UPYTH_MARKERS="$PREFIX_UPYTH_BUILD/markers/"
COPYPATH=""

b_log "Building micropython"

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
for patchfile in "$PREFIX_UPYTH_CONFIG"/patches/*.patch; do
	if [ ! -f "$PREFIX_UPYTH_MARKERS/$(basename "$patchfile").applied" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_UPYTH_SRC" -p1 < "$patchfile"   # FIXME: should be -p1
		touch "$PREFIX_UPYTH_MARKERS/$(basename "$patchfile").applied"
	fi
done
#
# Copy config file/s
#
while read -r line; do 
	arr=($line)
	cp -a "$PREFIX_UPYTH_CONFIG"/files/"${arr[0]}" "$PREFIX_UPYTH_SRC"/"${arr[1]}" && echo "Copied config: ${arr[0]} -> ${arr[1]}"
	echo "$PREFIX_UPYTH_SRC"/"${arr[1]}"
done < "$PREFIX_UPYTH_CONFIG"/"$DESTINATIONS_CFG"


#
# Micropython internal use stack/heap size (not actual application stack/heap)
# May be overwritten in _targets/build.project.*
#
if [[ ! $UPYTH_STACKSZ ]]; then
	UPYTH_STACKSZ="4096"
fi
if [[ ! $UPYTH_HEAPSZ ]]; then
	UPYTH_HEAPSZ="16384"
fi


#
# Architecture specific flags/values set
#
if [ "${TARGET_FAMILY}" = "armv7m7" ]
then
	export LDFLAGS_EXTRA=" -Wl,-q ${CFLAGS}"
	export STRIPFLAGS_EXTRA="--strip-unneeded"
elif [ "${TARGET_FAMILY}" = "ia32" ]
then
	export STRIPFLAGS_EXTRA="--strip-all"
else
	echo "Error: there is no micropython port for chosen architecture"
	exit 1
fi
export PHOENIX_MATH_ABSENT="expm1 log1p asinh acosh atanh erf tgamma lgamma copysign __sin __cos __tan __signbit"
export CFLAGS_EXTRA="${CFLAGS} -DUPYTH_STACKSZ=${UPYTH_STACKSZ} -DUPYTH_HEAPSZ=${UPYTH_HEAPSZ} "


#
# Build and install micropython binary
#
(cd "${PREFIX_UPYTH_SRC}/mpy-cross" && make all BUILD="${PREFIX_UPYTH_BUILD}" CROSS_COMPILE="${CROSS}")
(cd "${PREFIX_UPYTH_SRC}/ports/unix" && make all CROSS_COMPILE="${CROSS}")

cp -a "${PREFIX_UPYTH_SRC}/ports/unix/micropython" "$PREFIX_PROG"
cp -a "${PREFIX_UPYTH_SRC}/ports/unix/micropython" "$PREFIX_PROG_STRIPPED"
b_install "$PREFIX_PORTS_INSTALL/micropython" /bin/
