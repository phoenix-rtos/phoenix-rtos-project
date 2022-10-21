#!/bin/bash

set -o pipefail

if [ "$#" -ge 2 ]; then
    base="$1"
    head="$2"
else
    echo "need base sha and head sha"
    exit 1
fi

echo "base: $base"
echo "head: $head"

files=$(git diff "$base" "$head" --name-only --diff-filter=MA)
# shellcheck disable=SC2181
if [ "$?" -ne 0 ]; then
    echo "diff command failed"
    exit 1
fi


files=$(echo "$files" | egrep '\.c$|\.h$' | tr '\n' ' ')
# shellcheck disable=SC2181
if [ "$?" -eq 2 ]; then
    echo "egrep command failed"
    exit 1
fi

#echo "Files ${files}"

# Set github-actions variable
#echo "{files}={$files}" >> $GITHUB_OUTPUT
echo "::set-output name=files::$files"
echo -n "modified or added files: "
if [ "${files}" ]; then
    echo $files
else
    echo "there is no files with .c or .h suffix in diff"
fi
