#!/bin/bash

set -e

LZO=lzo-2.10

b_log "Building lzo"
PREFIX_LZO=${TOPDIR}/phoenix-rtos-ports/lzo
PREFIX_LZO_BUILD=${PREFIX_BUILD}/lzo
PREFIX_LZO_SRC=${PREFIX_LZO}/${LZO}

mkdir -p "$PREFIX_LZO_BUILD"

[ -f "$PREFIX_LZO/${LZO}.tar.gz" ] || wget http://www.oberhumer.com/opensource/lzo/download/${LZO}.tar.gz -P "$PREFIX_LZO"
[ -d "$PREFIX_LZO_SRC" ] || tar zxf "$PREFIX_LZO/${LZO}.tar.gz" -C "$PREFIX_LZO"

pushd $PREFIX_LZO_BUILD
if [ ! -z "$CLEAN" ]; then
	$PREFIX_LZO_SRC/configure --prefix=$PREFIX_LZO_BUILD --exec-prefix=$PREFIX_LZO_BUILD --libdir=$PREFIX_BUILD/lib/ --includedir=$PREFIX_BUILD/include/ \
		--host="${HOST_TARGET}-phoenix" CROSS="$CROSS" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS"
	make $CLEAN
fi

make
make install
popd

