#!/bin/bash

arch=$1
workdir=build/gcc4.9-$arch

cd $workdir || exit 1
env ASAN_OPTIONS=detect_leaks=1 cmake --build . --target unittest -- -j || exit 1
