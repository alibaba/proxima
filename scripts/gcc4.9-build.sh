#!/bin/bash

type=$1
arch=$2

eval $(ssh-agent -s)
chmod 600 .gitrsa && ssh-add .gitrsa

workdir=build/gcc4.9-$arch
mkdir -p $workdir
rsync -av . $workdir --exclude=build --exclude=.git || exit 1
cd $workdir
cmake -DCMAKE_BUILD_TYPE=$type -DENABLE_${arch^^}=ON . || exit 1
cmake --build . --target all -- -j || exit 1
