#!/bin/bash

set -e

UPYTH_VER="1.15"
UPYTH="micropython-${UPYTH_VER}"

PREFIX_UPYTH="${TOPDIR}/phoenix-rtos-ports/micropython"
PREFIX_UPYTH_BUILD="${PREFIX_BUILD}/micropython"
PREFIX_UPYTH_PATCHSET="${PREFIX_UPYTH}/${UPYTH}-patchset"


b_log "Building micropython"
pushd .
cd $PREFIX_UPYTH

#
# Download
#
[ -f "${UPYTH}.tar.xz" ] || wget https://github.com/micropython/micropython/releases/download/v${UPYTH_VER}/${UPYTH}.tar.xz
[ -d "${UPYTH}" ] || tar xf "${UPYTH}.tar.xz"


#
# Patch
#
if ! patch -R -p0 -s -f --dry-run < ${PREFIX_UPYTH_PATCHSET}/lib.patch
then
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/lib.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/py.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/mpy-cross.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/os.patch
fi


#
# Build and move micropython binary to filesystem root/bin/
#
cd ${UPYTH}/mpy-cross && make clean && make all BUILD=${PREFIX_UPYTH_BUILD} && cd ../..
cd ${UPYTH}/ports/unix && make clean && make all
mkdir -p "${PREFIX_FS}/root/bin/" && cp micropython "${PREFIX_FS}/root/bin/"

popd
