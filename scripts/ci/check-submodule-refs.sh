#!/bin/bash
# vim:noexpandtab:ts=4
#
# Verify that submodule gitlinks point to commits merged into the submodule's master branch.
#
# Submodule bumps in phoenix-rtos-project must reference commits that are already part of
# the submodule's origin/master history. Pointing a gitlink at a commit which only exists
# on a (possibly short-lived) topic branch breaks fresh clones as soon as that branch is
# deleted or rebased on merge, and silently pins the project to code that never landed.
#
# Usage:
#   check-submodule-refs.sh <base-rev> <head-rev>   # check gitlinks changed between the revisions
#   check-submodule-refs.sh --all [<rev>]           # check every gitlink at <rev> (default: HEAD)
#
# Environment:
#   SUBMODULE_BASE_BRANCH  - branch the commits must belong to (default: master)
#   SUBMODULE_CHECK_BACKEND - auto (default), api (GitHub compare API via gh) or git (fetch + merge-base)
#
# Exit codes:
#   0 - all moved gitlinks point to commits merged into the base branch
#   1 - at least one gitlink points elsewhere (or could not be verified)
#   2 - usage error

set -euo pipefail

# All lookups are anonymous reads of public repos. Make sure a repo we cannot read
# fails immediately instead of blocking CI on a prompt or opening a local askpass dialog.
export GIT_TERMINAL_PROMPT=0
export GIT_ASKPASS=true
export SSH_ASKPASS=true

BASE_BRANCH="${SUBMODULE_BASE_BRANCH:-master}"
BACKEND="${SUBMODULE_CHECK_BACKEND:-auto}"

TMPDIR_ROOT=""
FAILED=0
CHECKED=0

# ── helpers ──────────────────────────────────────────────────────────────────

log() { echo "  [submodule-refs] $*" >&2; }
warn() { echo "::warning::[submodule-refs] $*" >&2; }
err() { echo "::error::[submodule-refs] $*" >&2; }

# annotate <path> <message>
# GitHub-flavoured error annotation pinned to the submodule path.
annotate() {
	local path="$1" msg="$2"
	echo "::error file=${path},title=Submodule not on ${BASE_BRANCH}::${msg}"
}

summary() {
	[ -n "${GITHUB_STEP_SUMMARY:-}" ] || return 0
	echo "$*" >>"${GITHUB_STEP_SUMMARY}"
}

cleanup() {
	[ -n "${TMPDIR_ROOT}" ] && rm -rf -- "${TMPDIR_ROOT}"
	return 0
}
trap cleanup EXIT

usage() {
	sed -n '3,20p' "$0" >&2
	exit 2
}

# ── backend selection ────────────────────────────────────────────────────────

# gh is preferred when usable: a single API call per submodule, no history download.
select_backend() {
	case "${BACKEND}" in
	api | git) return 0 ;;
	auto) ;;
	*)
		err "unknown SUBMODULE_CHECK_BACKEND '${BACKEND}' (expected auto, api or git)"
		exit 2
		;;
	esac

	if command -v gh >/dev/null 2>&1 && { [ -n "${GH_TOKEN:-}" ] || [ -n "${GITHUB_TOKEN:-}" ] || gh auth status >/dev/null 2>&1; }; then
		BACKEND="api"
	else
		BACKEND="git"
	fi
}

# ── submodule metadata ───────────────────────────────────────────────────────

# gitmodules_url <rev> <path>
# Print the URL configured for the submodule checked out at <path> in <rev>.
gitmodules_url() {
	local rev="$1" path="$2" record key value name=""

	# -z records are "<key>\n<value>\0", so keys and values may contain spaces
	while IFS= read -r -d '' record; do
		key="${record%%$'\n'*}"
		value="${record#*$'\n'}"
		if [ "${value}" = "${path}" ]; then
			name="${key#submodule.}"
			name="${name%.path}"
			break
		fi
	done < <(git config --blob "${rev}:.gitmodules" -z --get-regexp '^submodule\..*\.path$' 2>/dev/null)

	[ -n "${name}" ] || return 1
	git config --blob "${rev}:.gitmodules" --get "submodule.${name}.url" 2>/dev/null
}

# normalize_slug <path>
# Collapse "." and ".." segments and print the result if it is exactly "owner/repo".
normalize_slug() {
	local path="$1" seg out=()

	local IFS=/
	for seg in ${path}; do
		case "${seg}" in
		"" | ".") ;;
		"..")
			[ "${#out[@]}" -gt 0 ] || return 1
			unset "out[$((${#out[@]} - 1))]"
			out=("${out[@]}")
			;;
		*) out+=("${seg}") ;;
		esac
	done

	[ "${#out[@]}" -eq 2 ] || return 1
	printf '%s/%s' "${out[0]}" "${out[1]}"
}

# github_slug <url>
# Convert an absolute GitHub remote URL into "owner/repo".
github_slug() {
	local url="$1"

	url="${url%/}"
	url="${url%.git}"
	case "${url}" in
	*github.com[:/]*) normalize_slug "$(printf '%s' "${url##*github.com}" | sed 's,^[:/],,')" ;;
	*) return 1 ;;
	esac
}

# parent_slug
# "owner/repo" of the superproject, used to resolve relative submodule URLs.
parent_slug() {
	local url

	if [ -n "${GITHUB_REPOSITORY:-}" ]; then
		normalize_slug "${GITHUB_REPOSITORY}"
		return
	fi

	url="$(git config --get remote.origin.url 2>/dev/null)" || return 1
	github_slug "${url}"
}

# url_to_slug <url>
# Convert a submodule URL into "owner/repo". Relative URLs (./foo, ../foo) are
# resolved against the superproject, the same way git itself resolves them.
url_to_slug() {
	local url="$1" parent

	url="${url%/}"
	url="${url%.git}"
	case "${url}" in
	./* | ../*)
		parent="$(parent_slug)" || return 1
		normalize_slug "${parent}/${url}"
		;;
	*) github_slug "${url}" ;;
	esac
}

# ── verification backends ────────────────────────────────────────────────────

# api_is_merged <slug> <sha>
# 0 - merged, 1 - not merged, 2 - inconclusive (network/auth/rate limit)
api_is_merged() {
	local slug="$1" sha="$2" status

	status="$(gh api "repos/${slug}/compare/${BASE_BRANCH}...${sha}" --jq '.status' 2>/dev/null)" || {
		# 404 for an unknown commit is a definitive answer, anything else is not.
		if gh api "repos/${slug}" --jq '.full_name' >/dev/null 2>&1; then
			return 1
		fi
		return 2
	}

	case "${status}" in
	identical | behind) return 0 ;;
	ahead | diverged) return 1 ;;
	*) return 2 ;;
	esac
}

# git_mirror <slug> <url>
# Print the path to a (lazily created) blobless mirror holding <base branch> as refs/heads/base.
# NOTE: callers run this in a command substitution, which disables set -e here - every step
# must therefore propagate its failure explicitly, otherwise we would hand out a mirror
# without the base branch and report bogus "not merged" results.
git_mirror() {
	local slug="$1" url="$2" dir

	if [ -z "${TMPDIR_ROOT}" ]; then
		TMPDIR_ROOT="$(mktemp -d)" || return 1
	fi
	dir="${TMPDIR_ROOT}/${slug//\//_}"

	# the marker is written only once the mirror is complete and usable
	if [ ! -e "${dir}/.mirror-ready" ]; then
		rm -rf -- "${dir}"
		{
			git init --bare -q "${dir}" &&
				git -C "${dir}" remote add origin "${url}" &&
				# blob filter: we only need the commit graph, not file contents
				git -C "${dir}" config remote.origin.promisor true &&
				git -C "${dir}" config remote.origin.partialclonefilter blob:none &&
				git -C "${dir}" fetch -q --no-tags origin "refs/heads/${BASE_BRANCH}:refs/heads/base" &&
				touch "${dir}/.mirror-ready"
		} >&2 || {
			rm -rf -- "${dir}"
			return 1
		}
	fi
	printf '%s' "${dir}"
}

# git_is_merged <slug> <url> <sha>
# 0 - merged, 1 - not merged, 2 - inconclusive
git_is_merged() {
	local slug="$1" url="$2" sha="$3" dir

	dir="$(git_mirror "${slug}" "${url}")" || return 2

	if ! git -C "${dir}" cat-file -e "${sha}^{commit}" 2>/dev/null; then
		# Not reachable from the base branch; try to fetch it directly to tell
		# "lives on another branch" apart from "does not exist at all".
		git -C "${dir}" fetch -q --no-tags origin "${sha}" 2>/dev/null || return 1
	fi

	git -C "${dir}" merge-base --is-ancestor "${sha}" refs/heads/base 2>/dev/null
}

# check_gitlink <path> <sha> <rev>
check_gitlink() {
	local path="$1" sha="$2" rev="$3" url slug fetch_url rc

	CHECKED=$((CHECKED + 1))

	if ! url="$(gitmodules_url "${rev}" "${path}")" || [ -z "${url}" ]; then
		annotate "${path}" "gitlink at '${path}' has no matching entry in .gitmodules"
		summary "| \`${path}\` | \`${sha}\` | :x: no .gitmodules entry |"
		FAILED=$((FAILED + 1))
		return 0
	fi

	if ! slug="$(url_to_slug "${url}")"; then
		warn "skipping '${path}': non-GitHub remote '${url}'"
		summary "| \`${path}\` | \`${sha}\` | :grey_question: skipped (non-GitHub remote) |"
		return 0
	fi

	# relative URLs are not clonable on their own - use the resolved slug instead
	case "${url}" in
	./* | ../*) fetch_url="https://github.com/${slug}" ;;
	*) fetch_url="${url}" ;;
	esac

	rc=0
	if [ "${BACKEND}" = "api" ]; then
		api_is_merged "${slug}" "${sha}" || rc=$?
		if [ "${rc}" -eq 2 ]; then
			warn "GitHub API check for ${slug} was inconclusive, falling back to git"
			rc=0
			git_is_merged "${slug}" "${fetch_url}" "${sha}" || rc=$?
		fi
	else
		git_is_merged "${slug}" "${fetch_url}" "${sha}" || rc=$?
	fi

	case "${rc}" in
	0)
		log "✓ ${path} @ ${sha} is on ${slug}/${BASE_BRANCH}"
		summary "| \`${path}\` | \`${sha}\` | :white_check_mark: on \`${BASE_BRANCH}\` |"
		;;
	1)
		annotate "${path}" "${path} is moved to ${sha}, which is not part of https://github.com/${slug} ${BASE_BRANCH}. Merge the submodule PR first, then bump the gitlink to the resulting ${BASE_BRANCH} commit."
		summary "| \`${path}\` | \`${sha}\` | :x: not on \`${BASE_BRANCH}\` |"
		FAILED=$((FAILED + 1))
		;;
	*)
		annotate "${path}" "could not verify ${path} @ ${sha} against ${slug} ${BASE_BRANCH}"
		summary "| \`${path}\` | \`${sha}\` | :x: verification failed |"
		FAILED=$((FAILED + 1))
		;;
	esac
}

# ── gitlink collection ───────────────────────────────────────────────────────

# moved_gitlinks <base-rev> <head-rev>
# Print "<new-sha> <path>" for every submodule added or moved between the revisions.
# The sha comes first because it can never contain a space, unlike the path.
moved_gitlinks() {
	local base="$1" head="$2" meta path dstmode dstsha status

	# raw diff records are "<meta>\t<path>", so split on the tab only
	while IFS=$'\t' read -r meta path; do
		read -r _ dstmode _ dstsha status <<<"${meta}"
		case "${dstmode}:${status}" in
		160000:A* | 160000:M*) printf '%s %s\n' "${dstsha}" "${path}" ;;
		esac
	done < <(git -c core.quotePath=false diff --raw --no-renames --abbrev=40 "${base}" "${head}")
}

# all_gitlinks <rev>
# Print "<sha> <path>" for every gitlink in the tree.
all_gitlinks() {
	local meta path type sha

	while IFS=$'\t' read -r meta path; do
		read -r _ type sha <<<"${meta}"
		if [ "${type}" = "commit" ]; then
			printf '%s %s\n' "${sha}" "${path}"
		fi
	done < <(git -c core.quotePath=false ls-tree -r "$1")
}

# ── main ─────────────────────────────────────────────────────────────────────

main() {
	local mode="range" base head rev gitlinks line

	case "${1:-}" in
	--all)
		mode="all"
		rev="${2:-HEAD}"
		;;
	-h | --help | "") usage ;;
	*)
		base="$1"
		head="${2:?Usage: check-submodule-refs.sh <base-rev> <head-rev>}"
		;;
	esac

	select_backend

	if [ "${mode}" = "all" ]; then
		rev="$(git rev-parse --verify "${rev}^{commit}")"
		log "checking all submodule gitlinks at ${rev} (backend: ${BACKEND})"
		gitlinks="$(all_gitlinks "${rev}")"
	else
		head="$(git rev-parse --verify "${head}^{commit}")"
		if ! base="$(git rev-parse --verify --quiet "${base}^{commit}")"; then
			# e.g. push event for a freshly created branch (before == 0000...)
			if base="$(git rev-parse --verify --quiet "${head}^1^{commit}")"; then
				warn "base revision not available, falling back to ${base}"
			else
				warn "base revision not available, checking all gitlinks at ${head}"
				rev="${head}"
				mode="all"
			fi
		fi

		if [ "${mode}" = "all" ]; then
			gitlinks="$(all_gitlinks "${rev}")"
		else
			base="$(git merge-base "${base}" "${head}")"
			log "checking submodule gitlinks moved in ${base}..${head} (backend: ${BACKEND})"
			gitlinks="$(moved_gitlinks "${base}" "${head}")"
			rev="${head}"
		fi
	fi

	if [ -z "${gitlinks}" ]; then
		log "no submodule gitlinks moved - nothing to check"
		summary "No submodule gitlinks were moved."
		return 0
	fi

	summary "### Submodule gitlinks"
	summary ""
	summary "| submodule | commit | result |"
	summary "| --- | --- | --- |"

	while IFS= read -r line; do
		[ -n "${line}" ] || continue
		check_gitlink "${line#* }" "${line%% *}" "${rev}"
	done <<<"${gitlinks}"

	summary ""
	if [ "${FAILED}" -gt 0 ]; then
		summary "**${FAILED} of ${CHECKED} submodule bump(s) do not point to \`${BASE_BRANCH}\`.**"
		err "${FAILED} of ${CHECKED} submodule bump(s) do not point to ${BASE_BRANCH}"
		return 1
	fi

	summary "All ${CHECKED} submodule bump(s) point to \`${BASE_BRANCH}\`."
	log "all ${CHECKED} submodule bump(s) point to ${BASE_BRANCH}"
	return 0
}

main "$@"
