#!/bin/bash
if [ $# -lt 2 ]; then
	echo 'Usage: $0 build_dir_name src_path'
	exit 1
fi
BUILD_DIR=${2}"/"$1
echo "build_dir: $BUILD_DIR"

set -xe
SE_DIR=${2}
RUN_DIR=${BUILD_DIR}/run
IT_DIR=${SE_DIR}/tests/integration
PYPROXIMA_DIR=${SE_DIR}/sdk/python
mkdir -p ${RUN_DIR}

export SRC_PATH=${2}
export BUILD_DIR_NAME=${1}
export PYTHONPATH=${PYPROXIMA_DIR}

cd $IT_DIR

# generate conf
(cd $PYPROXIMA_DIR; python3 setup.py bdist_wheel)
(cd $IT_DIR; python3 src/case/conf_replacer.py ${IT_DIR}/conf)

# start mysql
mysqld --defaults-file=${IT_DIR}/conf/my.cnf --initialize --log_error_verbosity --explicit_defaults_for_timestamp > ${RUN_DIR}/mysql.log 2>&1
mysqld --defaults-file=${IT_DIR}/conf/my.cnf --user=root >> ${RUN_DIR}/mysql.log 2>&1 &

ulimit -c unlimited

# start proxima_be
${BUILD_DIR}/bin/proxima_be -config ${IT_DIR}/conf/proxima_be.conf > ${RUN_DIR}/start.log 2>&1 &
sleep 10

# start mysql_repository
MYSQL_REPO_BIN=${BUILD_DIR}/bin/mysql_repository
${MYSQL_REPO_BIN} -config ${IT_DIR}/conf/mysql_repo.conf > ${RUN_DIR}/repo.log 2>&1 &
sleep 3

# run test
shift 
sh ${IT_DIR}/script/run_test.sh $@ || exit 1

