#!/bin/bash

set -e

WPA_SUPPLICANT=wpa_supplicant-2.9

b_log "Building wpa_supplicant"
PREFIX_WPA_SUPPLICANT="${TOPDIR}/phoenix-rtos-ports/wpa_supplicant"
PREFIX_WPA_SUPPLICANT_BUILD="${PREFIX_BUILD}/wpa_supplicant"
PREFIX_WPA_SUPPLICANT_SRC="${PREFIX_BUILD}/wpa_supplicant/${WPA_SUPPLICANT}"
PREFIX_WPA_SUPPLICANT_MARKERS="${PREFIX_WPA_SUPPLICANT_BUILD}/markers"
PREFIX_WPA_SUPPLICANT_INSTALL="${PREFIX_WPA_SUPPLICANT_BUILD}/install"

#
# Download and unpack
#
mkdir -p "$PREFIX_WPA_SUPPLICANT_BUILD"

[ -f "${PREFIX_WPA_SUPPLICANT}/${WPA_SUPPLICANT}.tar.gz" ] || wget https://w1.fi/releases/${WPA_SUPPLICANT}.tar.gz -P "$PREFIX_WPA_SUPPLICANT" --no-check-certificate
[ -d "$PREFIX_WPA_SUPPLICANT_SRC" ] || tar zxf "${PREFIX_WPA_SUPPLICANT}/${WPA_SUPPLICANT}.tar.gz" -C "$PREFIX_WPA_SUPPLICANT_BUILD"

#
# Apply patches
#
mkdir -p "$PREFIX_WPA_SUPPLICANT_MARKERS"

for patchfile in "${PREFIX_WPA_SUPPLICANT}"/patches/*.patch; do
	if [ ! -f "${PREFIX_WPA_SUPPLICANT_MARKERS}/$(basename "$patchfile").applied" ]; then
		echo "applying patch: $patchfile"
		patch -d "$PREFIX_WPA_SUPPLICANT_SRC" -p1 < "$patchfile"
		touch "${PREFIX_WPA_SUPPLICANT_MARKERS}/$(basename "$patchfile").applied"
	fi
done

#
# Make
#
mkdir -p "$PREFIX_WPA_SUPPLICANT_INSTALL"

pushd "${PREFIX_WPA_SUPPLICANT_SRC}/wpa_supplicant"
cp -a "$PREFIX_WPA_SUPPLICANT/config" .config
make install DESTDIR="$PREFIX_WPA_SUPPLICANT_INSTALL" LIBDIR="/lib" INCDIR="/include" BINDIR="/bin"
popd

cp -a "${PREFIX_WPA_SUPPLICANT_INSTALL}/bin/wpa_cli" "${PREFIX_PROG}/wpa_cli"
cp -a "${PREFIX_WPA_SUPPLICANT_INSTALL}/bin/wpa_supplicant" "${PREFIX_PROG}/wpa_supplicant"
"${CROSS}strip" -s "${PREFIX_PROG}/wpa_cli" -o "${PREFIX_PROG_STRIPPED}/wpa_cli"
"${CROSS}strip" -s "${PREFIX_PROG}/wpa_supplicant" -o "${PREFIX_PROG_STRIPPED}/wpa_supplicant"
b_install "${PREFIX_PORTS_INSTALL}/wpa_cli" /usr/bin
b_install "${PREFIX_PORTS_INSTALL}/wpa_supplicant" /usr/bin
