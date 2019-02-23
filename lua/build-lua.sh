LUA=lua-5.3.5

log_build "Building lua"
LUA_BUILD_DIR=$(pwd)/build/lua
LUA_SRC_DIR=$LUA_BUILD_DIR/${LUA}
mkdir -p "$LUA_BUILD_DIR"

if [ ! -z "$CLEAN_TARGET" ]; then
    [ -f "$LUA_BUILD_DIR/${LUA}.tar.gz" ] || wget https://www.lua.org/ftp/lua-5.3.5.tar.gz -P "$LUA_BUILD_DIR"
    if [ ! -d "$LUA_SRC_DIR" ]; then
		tar zxf "$LUA_BUILD_DIR/${LUA}.tar.gz" -C "$LUA_BUILD_DIR"
	fi

	cd "$LUA_SRC_DIR"
	cp $TOPDIR/lua/Makefile src/
fi
make clean posix
cd $TOPDIR
${CROSS}strip -s $LUA_SRC_DIR/src/lua -o $PREFIX_PROG_STRIPPED/lua
${CROSS}strip -s $LUA_SRC_DIR/src/luac -o $PREFIX_PROG_STRIPPED/luac
cp $LUA_SRC_DIR/src/lua $PREFIX_PROG/lua
cp $LUA_SRC_DIR/src/luac $PREFIX_PROG/luac
install_bin "$PREFIX_PROG_STRIPPED/lua" /bin
install_bin "$PREFIX_PROG_STRIPPED/luac" /bin