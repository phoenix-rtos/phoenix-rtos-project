#!/bin/bash

set -e

if [ "$(uname)" = "Darwin" ]; then
	b_log "Building mtd-utils"

	PREFIX_MTD_UTILS="${PREFIX_PROJECT}/mtd-utils"
	PREFIX_MTD_UTILS_BUILD="${PREFIX_PROJECT}/_build/host-generic-pc/mtd-utils"

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

	mkdir -p "${PREFIX_PROJECT}/_build/host-generic-pc/prog"
	mkdir -p "${PREFIX_PROJECT}/_build/host-generic-pc/prog.stripped"
	cp -a mkfs.jffs2 "${PREFIX_PROJECT}/_build/host-generic-pc/prog/"
	cp -a mkfs.jffs2 "${PREFIX_PROJECT}/_build/host-generic-pc/prog.stripped/"
	popd
fi
