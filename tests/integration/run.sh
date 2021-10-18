#!/bin/bash
mode='docker'
if [ $# -eq 1 ] && [ $1 = "local" ]; then
    mode=$1
fi

build_dir_name=`basename $(pwd)`

arch=$(uname -m)
if [ $arch = "aarch64" ]; then
	image=ghcr.io/alibaba/proxima-be-ci-aarch64:latest
else
	image=ghcr.io/alibaba/proxima-be-ci:latest
fi

if [ "${mode}" = "docker" ]; then
    src_path="/v"
    if [ $# -eq 2 ] || [ $# -ge 1 ] && [ $1 = "docker" ]; then
	shift
    fi
    (sudo docker run -it -v ${PWD}/..:${src_path} --net=none $image bash -x ${src_path}/tests/integration/script/setup_ci.sh ${build_dir_name} ${src_path} $@) || exit 1
#    sudo docker run -it -v ${PWD}/..:${src_path} --net=host $image bash
else
    src_path="/drone/src"
    (sh ${src_path}/tests/integration/script/setup_ci.sh build/${build_dir_name} ${src_path}) || exit 1
fi
