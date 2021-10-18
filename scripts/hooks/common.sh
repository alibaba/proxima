#!/bin/bash
check_clang_version() {
	if ! hash clang-format 2> /dev/null; then
		echo 'clang-format not installed'
		echo 'Maybe run `yum install -y ob-clang`'
		exit
	fi
	version=$(clang-format --version | awk '{print $3}' | awk -F. '{print $1}')
	if [ "$version" -le 5 ]; then
		echo 'clang-format too old'
		echo 'Maybe run `yum install -y ob-clang`'
		exit
	fi
}

ignore_merge() {
	# ignore merge request
	if git rev-parse MERGE_HEAD >/dev/null 2>&1; then
		exit
	fi
}

CPP_PATTERN="(src|tests)/.*\.(c|h|cpp|cc)$"
