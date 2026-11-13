#!/bin/bash
#
# Copies the Rust toolchain pinned by RUST-TOOLCHAIN to cache. Prints the
# resulting toolchain path.
#
# --tests: the standard library's tests built with that toolchain instead (the
# rust-toolchain-tests image of the same tag; scripts/rust/test-link.sh)
#
# RUST_TOOLCHAIN_IMAGE: overrides the RUST-TOOLCHAIN pin
# RUST_TOOLCHAIN_CACHE: default: ~/.cache/phoenix-rtos/rust-toolchain
#
# Copyright 2026 Phoenix Systems
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -euo pipefail

PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

tests=n
[ "${1:-}" != --tests ] || tests=y

image="${RUST_TOOLCHAIN_IMAGE:-$(sed -e 's/#.*//' -e 's/[[:space:]]//g' -e '/^$/d' "$PROJECT/RUST-TOOLCHAIN" | head -n 1)}"
if [ -z "$image" ]; then
	echo "toolchain.sh: $PROJECT/RUST-TOOLCHAIN names no image" >&2
	exit 1
fi

src=/opt/phoenix/rust done=bin/rustc
if [ "$tests" = y ]; then
	image="${image/rust-toolchain:/rust-toolchain-tests:}"
	src=/opt/phoenix/rust-tests done=PHOENIX-TOOLCHAIN.json
fi

cache="${RUST_TOOLCHAIN_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/phoenix-rtos/rust-toolchain}"
dir="$cache/$(printf '%s' "$image" | tr '/:@' '___')"

if [ ! -e "$dir/$done" ]; then
	mkdir -p "$cache"
	tmp="$(mktemp -d "$cache/.tmp.XXXXXX")" id=
	# also on an interrupt or a failed docker cp
	cleanup() {
		[ -z "$id" ] || docker rm "$id" >/dev/null || true
		rm -rf "$tmp"
	}
	trap cleanup EXIT
	id="$(docker create "$image" none)"
	docker cp -q "$id:$src" "$tmp/rust"
	rm -rf "$dir"
	mv "$tmp/rust" "$dir"
fi

echo "$dir"
