
DROPBEAR=dropbear-2018.76

b_log "Building dropbear"
PREFIX_DROPBEAR=${TOPDIR}/dropbear
PREFIX_DROPBEAR_BUILD=${PREFIX_BUILD}/dropbear
PREFIX_DROPBEAR_SRC=${PREFIX_DROPBEAR}/${DROPBEAR}

mkdir -p "$PREFIX_DROPBEAR_BUILD"

if [ ! -z "$CLEAN" ]; then
	[ -f "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" ] || wget http://matt.ucc.asn.au/dropbear/releases/${DROPBEAR}.tar.bz2 -P "${PREFIX_DROPBEAR}"
	[ -d "$PREFIX_DROPBEAR_SRC" ] || tar jxf "$PREFIX_DROPBEAR/${DROPBEAR}.tar.bz2" -C "${PREFIX_DROPBEAR}"

	cp $PREFIX_DROPBEAR/localoptions.h $PREFIX_DROPBEAR_SRC

	DROPBEAR_CFLAGS="-DENDIAN_LITTLE -DUSE_DEV_PTMX"

	cd ${PREFIX_DROPBEAR_SRC}

	DROPBEAR_CFLAGS="-Os -Wall -Wstrict-prototypes -g -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb -fomit-frame-pointer -mno-unaligned-access -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-z,max-page-size=0x1000 -DENDIAN_LITTLE -DUSE_DEV_PTMX"

#	echo $CFLAGS
	
#	${PREFIX_DROPBEAR_SRC}/configure CPPFLAGS="${DROPBEAR_CFLAGS} ${CFLAGS}" CFLAGS="${DROPBEAR_CFLAGS} ${CFLAGS}" LDFLAGS="${DROPBEAR_CFLAGS} ${LDFLAGS}" ARFLAGS="-r" --host="$TARGET_FAMILY" \
#		--target="$TARGET_FAMILY" CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as --includedir="${DROPBEAR_BUILD_DIR}/user-include" --oldincludedir="${DROPBEAR_BUILD_DIR}/user-include" \
#		--prefix="${DROPBEAR_BUILD_DIR}/rootfs" --program-prefix="" --libdir="${DROPBEAR_BUILD_DIR}/user-lib" --bindir="${DROPBEAR_BUILD_DIR}/rootfs/bin" --disable-zlib --enable-static \
#		--disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-harden

	LIBTOOL=arm-phoenix-libtool
	RANLIN=arm-phoenix-ranlib
	${PREFIX_DROPBEAR_SRC}/configure CPPFLAGS="${DROPBEAR_CFLAGS} ${CFLAGS}" CFLAGS="${DROPBEAR_CFLAGS} ${CFLAGS}" LDFLAGS="${DROPBEAR_CFLAGS} ${LDFLAGS}" ARFLAGS="-r" \
		--srcdir=$PREFIX_DROPBEAR_SRC \
		--host="$TARGET_FAMILY" --target="$TARGET_FAMILY" \
		--prefix="${FS}/root" --disable-zlib --enable-static \
		--disable-lastlog --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --disable-harden
	make -j 9

#--includedir="${PREFIX_DROPBEAR_BUILD}/user-include" --oldincludedir="${PREFIX_DROPBEAR_BUILD}/user-include" \
#--libdir="${PREFIX_DROPBEAR_BUILD}/user-lib"

fi

exit 0

make PROGRAMS="dropbear dbclient dropbearkey scp" -C "$DROPBEAR_SRC_DIR" -j 9 install
cd $TOPDIR
${CROSS}strip -s $DROPBEAR_BUILD_DIR/rootfs/sbin/dropbear -o $PREFIX_PROG_STRIPPED/dropbear
${CROSS}strip -s $DROPBEAR_BUILD_DIR/rootfs/bin/dbclient -o $PREFIX_PROG_STRIPPED/dbclient
${CROSS}strip -s $DROPBEAR_BUILD_DIR/rootfs/bin/scp -o $PREFIX_PROG_STRIPPED/scp
cp $DROPBEAR_BUILD_DIR/rootfs/sbin/dropbear $PREFIX_PROG/dropbear
cp $DROPBEAR_BUILD_DIR/rootfs/bin/dbclient $PREFIX_PROG/dbclient
cp $DROPBEAR_BUILD_DIR/rootfs/bin/scp $PREFIX_PROG/scp
install_bin "$PREFIX_PROG_STRIPPED/dropbear" /sbin
install_bin "$PREFIX_PROG_STRIPPED/dbclient" /usr/bin
install_bin "$PREFIX_PROG_STRIPPED/scp" /bin