#!/bin/bash

set -e

LUA=lua-5.3.5

b_log "Building lua"
PREFIX_LUA=${TOPDIR}/phoenix-rtos-ports/lua
PREFIX_LUA_SRC=${PREFIX_LUA}/${LUA}


[ -f "$PREFIX_LUA/${LUA}.tar.gz" ] || wget https://www.lua.org/ftp/lua-5.3.5.tar.gz -P "$PREFIX_LUA" --no-check-certificate
if [ ! -d "$PREFIX_LUA_SRC" ]; then
	tar zxf "$PREFIX_LUA/${LUA}.tar.gz" -C "$PREFIX_LUA"
	cp $PREFIX_LUA/Makefile $PREFIX_LUA_SRC/src/
fi

pushd $PREFIX_LUA_SRC
make $CLEAN posix
popd

${CROSS}strip -s $PREFIX_LUA_SRC/src/lua -o $PREFIX_PROG_STRIPPED/lua
${CROSS}strip -s $PREFIX_LUA_SRC/src/luac -o $PREFIX_PROG_STRIPPED/luac
cp $PREFIX_LUA_SRC/src/lua $PREFIX_PROG/lua
cp $PREFIX_LUA_SRC/src/luac $PREFIX_PROG/luac

b_install "$PREFIX_PORTS_INSTALL/lua" /bin
b_install "$PREFIX_PORTS_INSTALL/luac" /bin
