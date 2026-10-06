#!/bin/bash
# vim:noexpandtab:ts=4
#
# Check that the submodule gitlinks moved in <base-rev>..<head-rev> - and the gitlinks
# nested below them - point to commits already merged into the branch .gitmodules declares
# for them (master if unset). A gitlink left on a topic branch breaks fresh clones as soon
# as that branch is rebased or deleted, and pins the project to code that never landed.
#
# Only the submodules the diff actually moved get populated, so a PR that touches none
# costs nothing and a bump costs one shallow clone per moved submodule.
#
# NOTE: submodule paths containing spaces are not handled - awk splits on whitespace below.
#
# Usage: check-submodule-refs.sh <base-rev> <head-rev>

set -euo pipefail

[ $# -eq 2 ] || {
	echo "usage: ${0##*/} <base-rev> <head-rev>" >&2
	exit 2
}

head="$(git rev-parse --verify "$2^{commit}")"
# a push to a freshly created branch reports an all-zero base - fall back to the first parent
base="$(git rev-parse --verify --quiet "$1^{commit}" || git rev-parse --verify "${head}^1^{commit}")"
base="$(git merge-base "${base}" "${head}")"

# Populating submodules and reading submodule.<name>.branch both go through the working
# tree, not <head-rev>, so the two have to agree - the workflow checks out the PR head for
# exactly this reason. Bail out loudly rather than silently checking the wrong commits.
[ "$(git rev-parse HEAD)" = "${head}" ] || {
	echo "::error::working tree is at $(git rev-parse HEAD), expected ${head}" >&2
	exit 2
}

# gitlinks (mode 160000) added, modified or replacing a regular file
mapfile -t moved < <(git diff --raw --no-renames "${base}" "${head}" |
	awk '$2 == "160000" && $5 ~ /^[AMT]/ { print $6 }')
[ "${#moved[@]}" -gt 0 ] || exit 0

git submodule update --quiet --init --recursive --depth 1 -- "${moved[@]}"

fails="$(mktemp)"
export fails
trap 'rm -f -- "${fails}"' EXIT

# $name, $toplevel and $displaypath are set by foreach, so the body stays single-quoted
# shellcheck disable=SC2016
#
# No "exit 1" inside the loop - that aborts the whole walk at the first bad submodule.
# Failures are recorded in $fails instead, so one run reports every bad bump.
git submodule foreach --quiet --recursive '
	branch=$(git config -f "$toplevel/.gitmodules" submodule."$name".branch || echo master)

	# --unshallow drops the shallow graft, without which merge-base cannot connect HEAD to
	# the branch tip and reports commits that ARE merged as unmerged. --filter=tree:0 keeps
	# that cheap by fetching commit objects only. Both are needed, do not drop either.
	git fetch -q --unshallow --filter=tree:0 --no-tags origin "$branch" 2>/dev/null ||
		git fetch -q --filter=tree:0 --no-tags origin "$branch"

	if git merge-base --is-ancestor HEAD FETCH_HEAD; then
		echo "ok    $displaypath @ $(git rev-parse HEAD) is on $branch"
	else
		echo "::error file=$displaypath,title=Submodule not on $branch::$displaypath is moved to $(git rev-parse HEAD), which is not merged into $branch of $(git config --get remote.origin.url). Merge the submodule PR first, then bump the gitlink to the resulting $branch commit."
		echo "$displaypath" >>"$fails"
	fi
'

[ ! -s "${fails}" ] || {
	echo "::error::$(wc -l <"${fails}") submodule bump(s) do not point to their base branch"
	exit 1
}
