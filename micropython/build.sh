#!/bin/bash

set -e

UPYTH_FULLHASH="7f366a2190825555c16f57f5dfd4d0d57efd7c1f"
UPYTH="micropython-master"

PREFIX_UPYTH="${TOPDIR}/phoenix-rtos-ports/micropython"
PREFIX_UPYTH_BUILD="${PREFIX_BUILD}/micropython"
PREFIX_UPYTH_PATCHSET="${PREFIX_UPYTH}/${UPYTH}-patchset"


b_log "Building micropython"
pushd .
cd $PREFIX_UPYTH

#
# Download
#
if [[ ! -d ${UPYTH} ]]
then
	rm -rf "${UPYTH}" && rm -rf "${PREFIX_UPYTH_PATCHSET}"
	git clone https://github.com/micropython/micropython.git ${UPYTH}
	(cd ${UPYTH} && git reset --hard ${UPYTH_FULLHASH})

	#Submodules bloat the size of micropython source code (up to several GB!). 
	#Until submodules config feature is absent this line should not be uncommented!
	#(cd ${UPYTH} && && git submodule update --init --recursive)

fi

#
# Patch
#
if [[ ! -d "${PREFIX_UPYTH_PATCHSET}" ]]
then
	rm -rf ${PREFIX_UPYTH_PATCHSET}
	tar xzf "${UPYTH}-patchset.tar.gz"

	mv ${PREFIX_UPYTH}/${UPYTH}/ports/unix ${PREFIX_UPYTH}/${UPYTH}/_unix
	rm -rf ${PREFIX_UPYTH}/${UPYTH}/ports/*
	mv ${PREFIX_UPYTH}/${UPYTH}/_unix ${PREFIX_UPYTH}/${UPYTH}/ports/unix

	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/lib.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/py.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/mpy-cross.patch
	patch -s -p0 < ${PREFIX_UPYTH_PATCHSET}/os.patch

	mv ${PREFIX_UPYTH}/${UPYTH}/ports/unix ${PREFIX_UPYTH}/${UPYTH}/ports/phoenix
fi

#
# Build and move micropython binary to filesystem root/bin/
#
cd ${UPYTH}/mpy-cross && make clean && make all BUILD=${PREFIX_UPYTH_BUILD} && cd ../..
cd ${UPYTH}/ports/phoenix && make clean && make all
mkdir -p "${PREFIX_FS}/root/bin/" && cp micropython "${PREFIX_FS}/root/bin/"

popd
