PCRE=pcre-8.42

b_log "Building pcre"
PREFIX_PCRE=pcre
PREFIX_PCRE_BUILD=$PREFIX_BUILD/pcre
PREFIX_PCRE_SRC=$PREFIX_PCRE/${PCRE}
PREFIX_PCRE_SRC=$(realpath $PREFIX_PCRE_SRC)

mkdir -p "$PREFIX_PCRE_BUILD"

#if [ ! -z "$CLEAN" ]; then
#	[ -f "$PREFIX_PCRE/${PCRE}.tar.bz2" ] || wget http://ftp.pcre.org/pub/pcre/${PCRE}.tar.bz2 -P "$PREFIX_PCRE"

#	if [ ! -d "$PREFIX_PCRE_SRC" ]; then
#		tar jxf "$PREFIX_PCRE/${PCRE}.tar.bz2" -C "$PREFIX_PCRE_SRC"
#	fi

#	cd "$PREFIX_PCRE_SRC"
#	PCRE_CFLAGS="-Os -Wall -Wstrict-prototypes -g -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb -fomit-frame-pointer -mno-unaligned-access \
#		-fdata-sections -ffunction-sections"
#	PCRE_LDFLAGS="-z max-page-size=0x1000"
#echo $CFLAGS
#exit 0
#	./configure CFLAGS="$PCRE_CFLAGS" LDFLAGS="${PCRE_LDFLAGS}" ARFLAGS="-r" --enable-static --disable-shared --host="$TARGET_FAMILY" --target="$TARGET_FAMILY" \
#		--disable-cpp CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld AS=${CROSS}as LIBOOL=${CROSS}libtool --libdir=${PREFIX_PCRE_BUILD}/user-lib --with-gnu-ld=yes
#		--prefix="$PREFIX_PCRE_BUILD" --exec-prefix="$PREFIX_PCRE_BUILD"
#--libdir="${PREFIX_PCRE_BUILD}" \
#		--includedir="$PREFIX_PCRE_BUILD"

#echo $CC $AS $LD $AR
#exit 0
#	./configure CC=$CC CFLAGS="$CFLAGS" AS=$AS AR=$AR LD=$LD LIBTOOL=${CROSS}libtool --disable-cpp --enable-static --disable-shared --host="$TARGET_FAMILY" --target="$TARGET_FAMILY" \
#		--prefix=${PREFIX_BUILD}
#fi
#make clean
#make -C "$PREFIX_PCRE_SRC" clean all install

#exit 0
#cd $TOPDIR


b_log "Building busybox"


./busybox/build.sh ${CLEAN}
