#!/bin/bash

set -e

if [ "$(uname)" = "Darwin" ]; then
	b_log "Building mtd-utils"

	PREFIX_MTD_UTILS="${PREFIX_PROJECT}/mtd-utils"
	PREFIX_MTD_UTILS_BUILD="${PREFIX_BUILD_HOST}/mtd-utils"

	mkdir -p "$PREFIX_MTD_UTILS_BUILD"

	pushd "$PREFIX_MTD_UTILS_BUILD"
	if [ ! -f "${PREFIX_BUILD_HOST}/prog.stripped/mkfs.jffs2" ]; then
		tar xzvf "$PREFIX_MTD_UTILS/mtd-utils-patched.tar.gz"

		if [ -f Makefile ]; then
			make distclean
		fi

		./autogen.sh
		CFLAGS="" LDFLAGS="" CC=gcc ./configure --without-ubifs --without-lzo --without-xattr
 	fi

	make mkfs.jffs2

	mkdir -p "${PREFIX_BUILD_HOST}/prog"
	mkdir -p "${PREFIX_BUILD_HOST}/prog.stripped"
	cp -a mkfs.jffs2 "${PREFIX_BUILD_HOST}/prog/"
	cp -a mkfs.jffs2 "${PREFIX_BUILD_HOST}/prog.stripped"
	popd
fi
