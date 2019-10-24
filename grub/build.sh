#!/bin/bash

set -e
GRUB=grub-2.04

echo "Building $GRUB"
PREFIX_GRUB=${TOPDIR}/grub
PREFIX_GRUB_BUILD=$PREFIX_BUILD/grub
PREFIX_GRUB_SRC=$PREFIX_GRUB/${GRUB}
PREFIX_GRUB_MARKERS=$PREFIX_GRUB_BUILD/markers

echo $PREFIX_GRUB_SRC

# Download and unpack
mkdir -p $PREFIX_GRUB_BUILD $PREFIX_GRUB_MARKERS
[ -f "$PREFIX_GRUB/${GRUB}.tar.xz" ] || wget "https://ftp.gnu.org/gnu/grub/${GRUB}.tar.xz" -P $PREFIX_GRUB --no-check-certificate
[ -d $PREFIX_GRUB_SRC ] || ( tar Jxf "$PREFIX_GRUB/${GRUB}.tar.xz" -C $PREFIX_GRUB && find -P $PREFIX_GRUB_MARKERS -size 0 -type f -name "*.applied" -delete )

# Apply patches
for patchfile in $PREFIX_GRUB/*.patch; do
	if [ ! -f "$PREFIX_GRUB_MARKERS/$(basename $patchfile.applied)" ]; then
		echo "applying patch: $patchfile"
		patch -d $PREFIX_GRUB_SRC -p1 < $patchfile
		touch "$PREFIX_GRUB_MARKERS/$(basename $patchfile.applied)"
	fi
done

pushd $PREFIX_GRUB_SRC
env -i PATH=$PATH ./configure	\
	--prefix=$PREFIX_GRUB_BUILD \
	--disable-grub-mkfont		\
	--disable-werror			\
	--disable-libzfs			\
	--disable-nls

# Clean
if [ ! -z $CLEAN ]; then
	make clean
fi
make install
popd
