#!/bin/bash
# vim:noexpandtab:ts=6
#
# Cross-repository dependency resolution for CI builds.
#
# When working on changes that span multiple submodules (e.g., kernel syscall API
# change + libphoenix adaptation), this script ensures all related branches are
# checked out together so CI builds and tests pass.
#
# Resolution order (later wins):
#   1. Same-branch matching: if other submodules have a branch with the same name
#	  as the PR branch, check them out automatically.
#   2. Depends-On directives in PR body (explicit overrides):
#	  - Depends-On: <submodule>:<branch_name>	 (checkout specific branch)
#	  - Depends-On: <submodule>#<pr_number>		(checkout specific PR)
#
# Usage:
#   resolve-deps.sh <project-root> <current-repo> <branch-name> [pr-body-file]
#
# Arguments:
#   project-root  - absolute path to phoenix-rtos-project checkout
#   current-repo  - name of the submodule being built (e.g., "libphoenix")
#   branch-name   - branch name of the current PR/push
#   pr-body-file  - optional file containing the PR body text
#
# Exit codes:
#   0 - success (some submodules may have been updated)
#   1 - fatal error (explicit Depends-On target not found)

set -euo pipefail


PROJECT_ROOT="${1:?Usage: resolve-deps.sh <project-root> <current-repo> <branch-name> [pr-body-file]}"
CURRENT_REPO="${2:?Usage: resolve-deps.sh <project-root> <current-repo> <branch-name> [pr-body-file]}"
BRANCH_NAME="${3:?Usage: resolve-deps.sh <project-root> <current-repo> <branch-name> [pr-body-file]}"
PR_BODY_FILE="${4:-}"

# ── helpers ──────────────────────────────────────────────────────────────────

log()   { echo "  [resolve-deps] $*" >&2; }
warn()  { echo "::warning::[resolve-deps] $*" >&2; }
err()   { echo "::error::[resolve-deps] $*" >&2; }

# validate_ref <ref-name>
# Ensure user-provided ref (branch name) is valid to prevent command injection.
validate_ref() {
	local ref="$1"
	if ! git check-ref-format --branch "${ref}" >/dev/null 2>&1; then
		warn "  ✗ invalid branch name '${ref}'"
		return 1 # FAIL
	fi
	return 0
}

# checkout_branch <submodule-path> <branch-name>
# Returns 0 on success, 1 if branch doesn't exist
checkout_branch() {
	local sub_path="$1" branch="$2"
	pushd "${PROJECT_ROOT}/${sub_path}" > /dev/null

	# Defense-in-depth: validate branch name before passing to git commands
	if ! git check-ref-format --branch "${branch}" >/dev/null 2>&1; then
		warn "  ✗ ${sub_path}: invalid branch name '${branch}'"
		popd > /dev/null
		return 1
	fi

	# Check if the branch exists on the remote
	if git fetch --depth=1 origin "${branch}" >/dev/null 2>&1; then
		git checkout FETCH_HEAD
		log "  ✓ ${sub_path}: checked out branch '${branch}' ($(git rev-parse --short HEAD))"
		popd > /dev/null
		return 0
	fi

	popd > /dev/null
	return 1
}

# checkout_pr <submodule-path> <pr-number>
# Returns 0 on success, 1 if PR doesn't exist
checkout_pr() {
	local sub_path="$1" pr_num="$2"
	pushd "${PROJECT_ROOT}/${sub_path}" > /dev/null

	if git fetch --depth=1 origin "pull/${pr_num}/head" 2>/dev/null; then
		git checkout FETCH_HEAD
		log "  ✓ ${sub_path}: checked out PR #${pr_num} ($(git rev-parse --short HEAD))"
		popd > /dev/null
		return 0
	fi

	popd > /dev/null
	return 1
}

# ── main ─────────────────────────────────────────────────────────────────────

# Skip dependency resolution for default branches
if [ "${BRANCH_NAME}" = "master" ] || [ "${BRANCH_NAME}" = "main" ]; then
	log "On default branch '${BRANCH_NAME}', skipping dependency resolution"
	exit 0
fi

# Validate branch name to prevent command injection
if ! validate_ref "${BRANCH_NAME}"; then
	err "Current branch name '${BRANCH_NAME}' contains invalid characters"
	exit 1
fi

log "Resolving cross-repo dependencies for branch '${BRANCH_NAME}' (repo: ${CURRENT_REPO})"

# Get list of submodule paths from .gitmodules
cd "${PROJECT_ROOT}"
readarray -t SUBMODULES < <(git config --file .gitmodules --get-regexp '^submodule\..*\.path$' | awk '{print $2}')

if [ ${#SUBMODULES[@]} -eq 0 ]; then
	warn "No submodules found in .gitmodules"
	exit 0
fi

# Build associative array for O(1) submodule lookups
declare -A IS_SUBMODULE
for sm in "${SUBMODULES[@]}"; do IS_SUBMODULE["$sm"]=1; done

# Initialize error counter before Depends-On parsing
ERRORS=0

# ── Step 1: Parse Depends-On directives from PR body ────────────────────────

# Associative arrays: key = submodule name, value = branch or PR number
declare -A EXPLICIT_BRANCH=()
declare -A EXPLICIT_PR=()

if [ -n "${PR_BODY_FILE}" ] && [ -f "${PR_BODY_FILE}" ]; then
	log "Parsing Depends-On directives from PR body..."
	while IFS= read -r line; do
		# Normalize: trim leading/trailing whitespace, handle \r
		line=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e 's/\r$//' <<< "${line}")

		# Match: Depends-On: <submodule>:<branch_name>
		if [[ "${line}" =~ ^[Dd]epends-[Oo]n:[[:space:]]*([^:#[:space:]]+):([^[:space:]]+)$ ]]; then
			repo="${BASH_REMATCH[1]}"
			branch="${BASH_REMATCH[2]}"

			if [ -z "${IS_SUBMODULE[${repo}]+_}" ]; then
				log "Depends-On: submodule '${repo}' not found in .gitmodules, skipping"
				continue
			fi

			if ! validate_ref "${branch}"; then
				err "Depends-On: branch name '${branch}' contains invalid characters"
				ERRORS=$((ERRORS + 1))
				continue
			fi

			EXPLICIT_BRANCH["${repo}"]="${branch}"
			log "  Found explicit dependency: ${repo} -> branch '${branch}'"

		# Match: Depends-On: <submodule>#<pr_number>
		elif [[ "${line}" =~ ^[Dd]epends-[Oo]n:[[:space:]]*([^:#[:space:]]+)#([0-9]+)$ ]]; then
			repo="${BASH_REMATCH[1]}"
			pr_num="${BASH_REMATCH[2]}"

			if [ -z "${IS_SUBMODULE[${repo}]+_}" ]; then
				log "Depends-On: submodule '${repo}' not found in .gitmodules, skipping"
				continue
			fi

			EXPLICIT_PR["${repo}"]="${pr_num}"
			log "  Found explicit dependency: ${repo} -> PR #${pr_num}"
		fi
	done < "${PR_BODY_FILE}"
fi

# ── Step 2: Resolve dependencies for each submodule ─────────────────────────

RESOLVED=0

for submodule in "${SUBMODULES[@]}"; do
	# Skip the repo that triggered this CI run
	if [ "${submodule}" = "${CURRENT_REPO}" ]; then
		continue
	fi

	# Priority 1: Explicit Depends-On branch
	if [ -n "${EXPLICIT_BRANCH[${submodule}]+_}" ]; then
		target_branch="${EXPLICIT_BRANCH[${submodule}]}"
		if checkout_branch "${submodule}" "${target_branch}"; then
			RESOLVED=$((RESOLVED + 1))
		else
			err "Depends-On: branch '${target_branch}' not found in ${submodule}"
			ERRORS=$((ERRORS + 1))
		fi
		continue
	fi

	# Priority 2: Explicit Depends-On PR
	if [ -n "${EXPLICIT_PR[${submodule}]+_}" ]; then
		target_pr="${EXPLICIT_PR[${submodule}]}"
		if checkout_pr "${submodule}" "${target_pr}"; then
			RESOLVED=$((RESOLVED + 1))
		else
			err "Depends-On: PR #${target_pr} not found in ${submodule}"
			ERRORS=$((ERRORS + 1))
		fi
		continue
	fi

	# Priority 3: Same-branch-name matching (automatic)
	if checkout_branch "${submodule}" "${BRANCH_NAME}"; then
		RESOLVED=$((RESOLVED + 1))
	fi
	# If branch doesn't exist in the other submodule, that's fine - use default
done

# ── Summary ──────────────────────────────────────────────────────────────────

log "Dependency resolution complete: ${RESOLVED} submodule(s) updated, ${ERRORS} error(s)"

if [ "${ERRORS}" -gt 0 ]; then
	err "Some explicit Depends-On targets could not be resolved"
	exit 1
fi

exit 0
