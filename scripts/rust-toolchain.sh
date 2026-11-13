#!/bin/bash
#
# Copies the Rust toolchain pinned by RUST-TOOLCHAIN to cache. Prints the
# resulting toolchain path.
#
# RUST_TOOLCHAIN_IMAGE: overrides the RUST-TOOLCHAIN pin
# RUST_TOOLCHAIN_CACHE: default: ~/.cache/phoenix-rtos/rust-toolchain
#
# Copyright 2026 Phoenix Systems
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -euo pipefail

PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

image="${RUST_TOOLCHAIN_IMAGE:-$(sed -e 's/#.*//' -e 's/[[:space:]]//g' -e '/^$/d' "$PROJECT/RUST-TOOLCHAIN" | head -1)}"
if [ -z "$image" ]; then
	echo "rust-toolchain.sh: $PROJECT/RUST-TOOLCHAIN names no image" >&2
	exit 1
fi

cache="${RUST_TOOLCHAIN_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/phoenix-rtos/rust-toolchain}"
dir="$cache/$(printf '%s' "$image" | tr '/:@' '___')"

if [ ! -x "$dir/bin/rustc" ]; then
	mkdir -p "$cache"
	tmp="$(mktemp -d "$cache/.tmp.XXXXXX")"
	trap 'rm -rf "$tmp"' EXIT
	id="$(docker create "$image" none)"
	docker cp -q "$id:/opt/phoenix/rust" "$tmp/rust" || { docker rm "$id" > /dev/null; exit 1; }
	docker rm "$id" > /dev/null
	rm -rf "$dir"
	mv "$tmp/rust" "$dir"
fi

echo "$dir"
