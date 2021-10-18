#!/bin/bash
if [ -d .git/hooks ]; then
	for f in pre-commit post-commit
	do
		ln -snf ../../scripts/hooks/$f .git/hooks/$f
	done
fi
