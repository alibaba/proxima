#!/bin/bash

export PATH=/usr/local/gcc-8.3.0/bin:/opt/cmake/bin/:$PATH
export LD_LIBRARY_PATH=/usr/local/gcc-8.3.0/lib64
export CC=/usr/local/gcc-8.3.0/bin/gcc
export CXX=/usr/local/gcc-8.3.0/bin/g++

$CC --version
type=$1
arch=$2

eval $(ssh-agent -s)
chmod 600 .gitrsa && ssh-add .gitrsa

workdir=build/gcc8.3-$arch
mkdir -p $workdir
rsync -av . $workdir --exclude=build --exclude=.git || exit 1
cd $workdir
cmake -DCMAKE_BUILD_TYPE=$type -DENABLE_${arch^^}=ON -DENABLE_LOCAL_INTEGRATION=ON. || exit 1
cmake --build . --target all -- -j || exit 1
