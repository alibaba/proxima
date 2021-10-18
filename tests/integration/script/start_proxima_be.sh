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

export PYTHONPATH=${SE_DIR}/python

ulimit -c unlimited

${BUILD_DIR}/bin/proxima_be -config ${IT_DIR}/conf/proxima_be.conf > ${RUN_DIR}/start.log 2>&1 &
sleep 2


