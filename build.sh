#!/bin/bash
#
# Shell script for building Phoenix-RTOS ports
#
# Copyright 2019 Phoenix Systems
# Author: Pawel Pisarczyk
#

export TARGET TARGET_FAMILY TOPDIR PREFIX_BUILD PREFIX_FS PREFIX_BOOT PREFIX_PROG PREFIX_PROG_STRIPPED PREFIX_A PREFIX_H CROSS CFLAGS LDFLAGS CC LD AR CLEAN MAKEFLAGS


[ "X${PORTS_BUSYBOX}" == "Xy" ] && ./phoenix-rtos-ports/busybox/build.sh

[ "X${PORTS_PCRE}" == "Xy" ] && ./phoenix-rtos-ports/pcre/build.sh

[ "X${PORTS_LIGHTTPD}" == "Xy" ] && ./phoenix-rtos-ports/lighttpd/build.sh

[ "X${PORTS_DROPBEAR}" == "Xy" ] && ./phoenix-rtos-ports/dropbear/build.sh

[ "X${PORTS_LUA}" == "Xy" ] && ./phoenix-rtos-ports/lua/build.sh

[ "X${PORTS_OPENSSL}" == "Xy" ] && ./phoenix-rtos-ports/openssl/build.sh

#[ "X${PORTS_OPENVPN}" == "Xy" ] && ./phoenix-rtos-ports/openvpn/build.sh
