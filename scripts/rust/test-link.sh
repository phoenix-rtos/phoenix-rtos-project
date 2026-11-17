#!/bin/bash
#
# Links a standard library test built by phoenix-rtos/rust's
# build-phoenix-tests.sh (unlinked: <test>.o, <test>.link, rlibs/) against
# this project's libphoenix, for TARGET:
#
#   scripts/rust/test-link.sh <tests>/<rust target> <test> <output>
#
# Run in the build environment (build.sh env) with RUST_ENABLE=y and
# RUST_TOOLCHAIN_DIR set.
#
# Copyright 2026 Phoenix Systems
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -euo pipefail

dir="$1" name="$2" out="$3"
PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

eval "$(make -s --no-print-directory -C "$PROJECT/_user" export-cargo-env)"
target_env="$(tr 'a-z-' 'A-Z_' <<<"$CARGO_BUILD_TARGET")"
rustflags_var="CARGO_TARGET_${target_env}_RUSTFLAGS"
linker_var="CARGO_TARGET_${target_env}_LINKER"
gcc_ld="-B$RUST_TOOLCHAIN_DIR/lib/rustlib/x86_64-unknown-linux-gnu/bin/gcc-ld"

# the project's link flags, as it passes them to rustc
read -ra rustflags <<<"${!rustflags_var}"
args=()
for ((i = 0; i < ${#rustflags[@]}; i++)); do
	case "${rustflags[i]}" in
	-C)
		i=$((i + 1))
		case "${rustflags[i]}" in
		link-arg=*) args+=("${rustflags[i]#link-arg=}") ;;
		linker-flavor=gnu-lld-cc) args+=(-fuse-ld=lld "$gcc_ld") ;;
		esac
		;;
	-L)
		i=$((i + 1))
		args+=("-L${rustflags[i]#native=}")
		;;
	esac
done

# then the test, its rlibs and the rest of rustc's link command, without the
# paths of the machine that built it
rlibs=() recorded=() skip=n
while read -r kind value; do
	if [ "$kind" = rlib ]; then
		rlibs+=("$dir/rlibs/$value")
	elif [ "$skip" = y ]; then
		skip=n
	else
		case "$value" in
		-L) skip=y ;;
		-Wl,-rpath,* | -Wl,-z,origin) ;;
		-fuse-ld=lld) recorded+=("$value" "$gcc_ld") ;;
		*) recorded+=("$value") ;;
		esac
	fi
done <"$dir/$name.link"

# libgcc last: helpers libphoenix needs that the test's compiler_builtins may lack
"${!linker_var}" "${args[@]}" "$dir/$name.o" -Wl,--start-group "${rlibs[@]}" -Wl,--end-group "${recorded[@]}" -lgcc -o "$out"
