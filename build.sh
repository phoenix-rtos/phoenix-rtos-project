#!/bin/bash
#
# Shell script for building Phoenix-RTOS ports
#
# Copyright 2019 Phoenix Systems
# Author: Pawel Pisarczyk
#

set -e

if [ $TARGET_FAMILY == "ia32" ]; then
	HOST_TARGET="i386"
else
	HOST_TARGET="$TARGET_FAMILY"
fi

export TARGET TARGET_FAMILY HOST_TARGET TOPDIR PREFIX_BUILD PREFIX_FS PREFIX_BOOT PREFIX_PROG PREFIX_PROG_STRIPPED PREFIX_A PREFIX_H CROSS CFLAGS LDFLAGS CC LD AR CLEAN MAKEFLAGS

if [[ -n "$PORTS_INSTALL_STRIPPED" && "$PORTS_INSTALL_STRIPPED" = "n" ]]; then
	export PREFIX_PORTS_INSTALL=$PREFIX_PROG
else
	export PREFIX_PORTS_INSTALL=$PREFIX_PROG_STRIPPED
fi


[ "X${PORTS_BUSYBOX}" == "Xy" ] && ./phoenix-rtos-ports/busybox/build.sh

[ "X${PORTS_PCRE}" == "Xy" ] && ./phoenix-rtos-ports/pcre/build.sh

[ "X${PORTS_OPENSSL}" == "Xy" ] && ./phoenix-rtos-ports/openssl/build.sh

[ "X${PORTS_LIGHTTPD}" == "Xy" ] && ./phoenix-rtos-ports/lighttpd/build.sh

[ "X${PORTS_DROPBEAR}" == "Xy" ] && ./phoenix-rtos-ports/dropbear/build.sh

[ "X${PORTS_LUA}" == "Xy" ] && ./phoenix-rtos-ports/lua/build.sh

[ "X${PORTS_LZO}" == "Xy" ] && ./phoenix-rtos-ports/lzo/build.sh

[ "X${PORTS_OPENVPN}" == "Xy" ] && ./phoenix-rtos-ports/openvpn/build.sh

[ "X${PORTS_CURL}" == "Xy" ] && ./phoenix-rtos-ports/curl/build.sh

[ "X${PORTS_JANSSON}" == "Xy" ] && ./phoenix-rtos-ports/jansson/build.sh

exit 0
