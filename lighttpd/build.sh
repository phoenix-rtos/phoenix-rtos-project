

LIGHTTPD=lighttpd-1.4.50

log_build "Building lighttpd"
LIGHTTPD_BUILD_DIR=$(pwd)/build/lighttpd/build
LIGHTTPD_ARCH_DIR=$(pwd)/build/lighttpd
LIGHTTPD_SRC_DIR=$LIGHTTPD_ARCH_DIR/${LIGHTTPD}
mkdir -p "$LIGHTTPD_BUILD_DIR"

if [ ! -z "$CLEAN_TARGET" ]; then
	[ -f "$LIGHTTPD_ARCH_DIR/${LIGHTTPD}.tar.gz" ] || wget https://download.lighttpd.net/lighttpd/releases-1.4.x/${LIGHTTPD}.tar.gz -P "$LIGHTTPD_ARCH_DIR"
	if [ ! -d "$LIGHTTPD_SRC_DIR" ]; then
		tar zxf "$LIGHTTPD_ARCH_DIR/${LIGHTTPD}.tar.gz" -C "$LIGHTTPD_ARCH_DIR"
		cp -R "lighttpd/patches" $LIGHTTPD_ARCH_DIR

		cd $LIGHTTPD_SRC_DIR
		for i in ${LIGHTTPD_ARCH_DIR}/patches/*.patch; do patch -t -p1 < $i; done
	fi

	cd $TOPDIR
	cat $FSDIR/etc/lighttpd.conf | grep mod_ | cut -d'"' -f2 | xargs -L1 -I{} echo "PLUGIN_INIT({})" > $LIGHTTPD_SRC_DIR/src/plugin-static.h

	cd $LIGHTTPD_SRC_DIR
	LIGHTTPD_CFLAGS="-I$PCRE_BUILD_DIR/user-include/ -Os -Wall -Wstrict-prototypes -g -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb \
		-fomit-frame-pointer -mno-unaligned-access -fdata-sections -ffunction-sections -DLIGHTTPD_STATIC -DPHOENIX"
	LIGHTTPD_LDFLAGS="-L$PCRE_BUILD_DIR/user-lib/ -z max-page-size=0x1000"

	./configure LIGHTTPD_STATIC=yes CFLAGS="${LIGHTTPD_CFLAGS} ${CFLAGS}" LDFLAGS="${LIGHTTPD_LDFLAGS} ${LDFLAGS}" AR_FLAGS="-r" -C --disable-ipv6 --disable-mmap --with-bzip2=no \
		--with-zlib=no --enable-shared=no --enable-static=yes --disable-shared --host="$TARGET_FAMILY" -target="$TARGET_FAMILY" CC=${CROSS}gcc \
		AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as --prefix="$LIGHTTPD_BUILD_DIR/rootfs" --libdir="$LIGHTTPD_BUILD_DIR/user-lib" --includedir="$LIGHTTPD_BUILD_DIR/user-include"

	sed -i 's/#define HAVE_MMAP 1//g' config.h
	sed -i 's/#define HAVE_MUNMAP 1//g' config.h
	sed -i 's/#define HAVE_GETRLIMIT 1//g' config.h
	sed -i 's/#define HAVE_SYS_POLL_H 1//g' config.h
	sed -i 's/#define HAVE_SIGACTION 1//g' config.h
	sed -i 's/#define HAVE_DLFCN_H 1//g' config.h

fi
make -C "$LIGHTTPD_SRC_DIR" -j 9 install
cd $TOPDIR
${CROSS}strip -s $LIGHTTPD_BUILD_DIR/rootfs/sbin/lighttpd -o $PREFIX_PROG_STRIPPED/lighttpd
cp $LIGHTTPD_BUILD_DIR/rootfs/sbin/lighttpd $PREFIX_PROG/lighttpd
install_bin "$PREFIX_PROG_STRIPPED/lighttpd" /sbin