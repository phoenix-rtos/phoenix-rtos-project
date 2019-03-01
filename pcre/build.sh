#!/bin/bash
PCRE=pcre-8.42

b_log "Building pcre"
PREFIX_PCRE=${TOPDIR}/phoenix-rtos-ports/pcre
PREFIX_PCRE_BUILD=${PREFIX_BUILD}/pcre
PREFIX_PCRE_SRC=${PREFIX_PCRE}/${PCRE}

#
# Download and unpack
#
mkdir -p "$PREFIX_PCRE_BUILD"

if [ ! -z "$CLEAN" ]; then
	[ -f "$PREFIX_PCRE/${PCRE}.tar.bz2" ] || wget http://ftp.pcre.org/pub/pcre/${PCRE}.tar.bz2 -P "$PREFIX_PCRE"
	[ -d "$PREFIX_PCRE_SRC" ] || tar jxf "$PREFIX_PCRE/${PCRE}.tar.bz2" -C "$PREFIX_PCRE"


	PCRE_CFLAGS="-Os -Wall -Wstrict-prototypes -g -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb -fomit-frame-pointer -mno-unaligned-access \
		-fdata-sections -ffunction-sections"

	PCRE_LDFLAGS="-z max-page-size=0x1000"

#
# Configure
#
	( cd ${PREFIX_PCRE_SRC} && ${PREFIX_PCRE_SRC}/configure CFLAGS="${PCRE_CFLAGS} ${CFLAGS}" LDFLAGS="${PCRE_LDFLAGS} ${LDFLAGS}" ARFLAGS="-r" --enable-static --disable-shared --host="$TARGET_FAMILY" --target="$TARGET_FAMILY" \
                --disable-cpp CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as --prefix="$PREFIX_PCRE_BUILD/rootfs" --libdir="${PREFIX_PCRE_BUILD}/user-lib" \
                --includedir="${PREFIX_PCRE_BUILD}/user-include" )
fi

#
# Make
#
make -C "$PREFIX_PCRE_SRC" CROSS_COMPILE="$CROSS" ${MAKEFLAGS} install
