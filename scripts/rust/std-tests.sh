#!/bin/bash
#
# Links and runs the standard library's tests that a list names:
#
#   TARGET=<target> scripts/rust/std-tests.sh [list]
#
# The default list is std-tests.list, or std-tests-nommu.list on targets
# without an MMU.
#
# Copyright 2026 Phoenix Systems
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -euo pipefail

PROJECT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# a path the container sees: relative to the project if inside it
container_path() {
	local path
	path="$(realpath "$1")"
	case "$path" in
	"$PROJECT"/*) echo "${path#"$PROJECT"/}" ;;
	*) echo "$path" ;;
	esac
}

if [ "${1:-}" != --in-env ]; then
	[ -n "${TARGET:-}" ] || {
		echo "std-tests.sh: TARGET is not set" >&2
		exit 1
	}
	export RUST_ENABLE=y
	list="${1:+$(container_path "$1")}"
	tests="$(container_path "$(RUST_TOOLCHAIN_CACHE="${RUST_TOOLCHAIN_CACHE:-$PROJECT/_build/rust-cache}" "$PROJECT/scripts/rust/toolchain.sh" --tests)")"
	read -ra devel <<<"${DOCKER_DEVEL:-$PROJECT/docker-devel.sh bash -c}"
	cd "$PROJECT"
	exec "${devel[@]}" "python3 -m pip install -q jinja2 && ./phoenix-rtos-build/build.sh env <<< './scripts/rust/std-tests.sh --in-env $tests ${list:-}'"
fi

# in the build environment
cd "$PROJECT"
dir="$2" list="${3:-}"

if [ -z "$list" ]; then
	list=scripts/rust/std-tests.list
	# shellcheck disable=SC2016 # $(HAVE_MMU) is make's
	if [ "$(make -s --no-print-directory -C _user --eval='print-have-mmu: ; @echo $(HAVE_MMU)' print-have-mmu)" = n ]; then
		list=scripts/rust/std-tests-nommu.list
	fi
fi

eval "$(make -s --no-print-directory -C _user export-cargo-env)"
# std's default thread stack on x86 and arm is 8 KiB, which some tests outgrow
case "$CARGO_BUILD_TARGET" in
i586-* | arm* | thumb*) export RUST_RUNNER_ENV="${RUST_RUNNER_ENV:-RUST_MIN_STACK=32768}" ;;
esac
dir="$dir/$CARGO_BUILD_TARGET"
runner_var="CARGO_TARGET_$(tr 'a-z-' 'A-Z_' <<<"$CARGO_BUILD_TARGET")_RUNNER"

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
export RUST_RUNNER_DIR="$work/runner"

failed=0
while read -r test args; do
	[ -n "$test" ] || continue
	read -ra argv <<<"$args"
	start=$SECONDS
	status=ok
	if [ ! -x "$work/$test" ] && ! "$PROJECT/scripts/rust/test-link.sh" "$dir" "$test" "$work/$test" >"$work/log" 2>&1; then
		status="FAILED (link)"
	elif ! "${!runner_var}" "$work/$test" "${argv[@]}" </dev/null >"$work/log" 2>&1; then
		status=FAILED
	fi
	result="$(tr -d '\r' <"$work/log" | grep -a '^test result:' | tail -1 || true)"
	line="$status: $test${args:+ $args} ($((SECONDS - start)) s) ${result#test result: }"
	echo "$line"
	if [ "$status" != ok ]; then
		failed=$((failed + 1))
		excerpt="$(tr -d '\r' <"$work/log" | grep -a -E '^test .* FAILED$|panicked at|crashed|error' | head -20 || true)"
		# none of those (a boot that did not reach the shell, a runner error): the log's end
		[ -n "$excerpt" ] || excerpt="$(tr -d '\r' <"$work/log" | tail -20)"
		# shellcheck disable=SC2001 # indents every line
		sed 's/^/    /' <<<"$excerpt"
	fi
	[ -z "${GITHUB_STEP_SUMMARY:-}" ] || echo "- $line" >>"$GITHUB_STEP_SUMMARY"
done < <(sed -e 's/#.*//' "$list")

[ "$failed" -eq 0 ]
