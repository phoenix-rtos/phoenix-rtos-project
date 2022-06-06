#!/bin/bash

set -e

if [ "$(uname)" = "Darwin" ]; then
	b_log "Building mtd-utils"

	PREFIX_MTD_UTILS="${TOPDIR}/mtd-utils"
	PREFIX_MTD_UTILS_BUILD="${TOPDIR}/_build/host-generic-pc/mtd-utils"

	mkdir -p "$PREFIX_MTD_UTILS_BUILD"

	pushd "$PREFIX_MTD_UTILS_BUILD"
	if [ -n "$CLEAN" ]; then
		tar xzvf "$PREFIX_MTD_UTILS/mtd-utils-patched.tar.gz"

		if [ -f Makefile ]; then
			make distclean
		fi

		./autogen.sh
		CFLAGS="" LDFLAGS="" CC=gcc ./configure --without-ubifs --without-lzo --without-xattr
 	fi

	make mkfs.jffs2
	
	mkdir -p "${TOPDIR}/_build/host-generic-pc/prog"
	mkdir -p "${TOPDIR}/_build/host-generic-pc/prog.stripped"
	cp mkfs.jffs2 "${TOPDIR}/_build/host-generic-pc/prog/"
	cp mkfs.jffs2 "${TOPDIR}/_build/host-generic-pc/prog.stripped/"
	popd
fi
