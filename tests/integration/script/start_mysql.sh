#!/bin/bash
if [ $# -ne 2 ]; then
	echo 'Usage: $0 build_dir_name src_path'
	exit 1
fi

BUILD_DIR=${2}"/"${1}
echo "build_dir: $BUILD_DIR"

set -xe
SE_DIR=${2}
RUN_DIR=${BUILD_DIR}/run
IT_DIR=${SE_DIR}/tests/integration

# start mysql
mysqld --defaults-file=${IT_DIR}/conf/my.cnf --user=root >> ${RUN_DIR}/mysql.log 2>&1 &

sleep 3
