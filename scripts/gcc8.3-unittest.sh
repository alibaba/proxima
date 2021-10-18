#!/bin/bash

export PATH=/usr/local/gcc-8.3.0/bin:/opt/cmake/bin/:$PATH
export LD_LIBRARY_PATH=/usr/local/gcc-8.3.0/lib64
export CC=/usr/local/gcc-8.3.0/bin/gcc
export CXX=/usr/local/gcc-8.3.0/bin/g++

arch=$1
workdir=build/gcc8.3-$arch

cd $workdir || exit 1
env ASAN_OPTIONS=detect_leaks=1,verify_asan_link_order=0 cmake --build . --target unittest -- -j || exit 1
