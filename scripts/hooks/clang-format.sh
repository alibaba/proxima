#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
. $DIR/common.sh

ignore_merge

check_clang_version

updated=
for file in `git show --pretty="format:" --name-only HEAD` ; do
	if ! [[ "$file" =~ $CPP_PATTERN ]]; then
		#echo "non cpp $file"
		continue
	fi
	# ignore deleted file
	if ! [ -f $file ]; then
		#echo "deleted $file"
		continue
	fi
	# ignore modified file
	if [ -n "`git status -s $file `" ]; then
		#echo "modified $file"
		continue
	fi
	clang-format -i "${file}"
	updated="${file} $updated"
done
if [ -n "$updated" ]; then
	if [ -n "`git status -s $updated `" ]; then
		git commit --amend -C HEAD --no-verify $updated
	fi
fi

