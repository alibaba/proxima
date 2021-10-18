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

#lsof -i :16000 | grep LISTEN | awk '{print $2}' | xargs kill -s SIGUSR2
ps auxwww | grep proxima_be | grep proxima_be.conf | grep -v grep | awk '{print $2}' | xargs kill -s SIGUSR2

sleep 3

ulimit -c unlimited

${BUILD_DIR}/bin/proxima_be -config ${IT_DIR}/conf/proxima_be.conf > ${RUN_DIR}/start.log 2>&1 &
sleep 2


