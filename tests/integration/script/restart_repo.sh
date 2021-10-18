#!/bin/bash
if [ $# -ne 1 ]; then
	echo 'Usage: $0 build_dir_name'
	exit 1
fi
BUILD_DIR=/v/$1
echo "build_dir: $BUILD_DIR"
set -xe
SE_DIR=/v
RUN_DIR=${BUILD_DIR}/run
IT_DIR=${SE_DIR}/tests/integration

export PYTHONPATH=${SE_DIR}/python

ps auxwww | grep 'bin/mysql_repository' | grep -v grep | awk '{print $2}' | xargs kill -s SIGUSR2
sleep 1

ulimit -c unlimited

MYSQL_REPO_BIN=${BUILD_DIR}/bin/mysql_repository
${MYSQL_REPO_BIN} -config ${IT_DIR}/conf/mysql_repo.conf > ${RUN_DIR}/repo.log 2>&1 &
sleep 2


