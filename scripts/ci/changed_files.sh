#!/bin/bash

set -o pipefail

if [ "$#" -eq 0 ]; then
	files=$(git diff HEAD^ --name-only --diff-filter=MA)
elif [ "$#" -eq 2 ]; then
	echo "base: $1"
	echo "head: $2"
	files=$(git diff "$1" "$2" --name-only --diff-filter=MA)
else
    echo "need base sha and head sha"
    exit 1
fi

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
