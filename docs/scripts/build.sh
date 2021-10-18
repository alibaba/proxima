#!/bin/bash

export PATH=/usr/local/gcc-8.3.0/bin:/opt/cmake/bin/:$PATH
export LD_LIBRARY_PATH=/usr/local/gcc-8.3.0/lib64
export CC=/usr/local/gcc-8.3.0/bin/gcc
export CXX=/usr/local/gcc-8.3.0/bin/g++

$CC --version

# Install hugo
tar -xvf hugo_extended_0.87.0_Linux-64bit.tar.gz
./hugo version

# Install nodejs
tar -xvf node-v14.17.5-linux-x64.tar.xz
sudo mkdir -p /usr/local/lib/node
sudo mv node-v14.17.5-linux-x64 /usr/local/lib/node/nodejs
export NODEJS_HOME=/usr/local/lib/node/nodejs
export PATH=$NODEJS_HOME/bin:$PATH
node -v
npm version

# Install SCSS
cd /drone/src/docs/themes/
sudo rm -rf /drone/src/docs/themes/docsy
sudo cp -rf /home/admin/proxima/data/submodules/docsy.tar.gz /drone/src/docs/themes/
sudo tar -zxvf /drone/src/docs/themes/docsy.tar.gz
cd /drone/src/docs
sudo npm install -D --save autoprefixer
sudo npm install -D --save postcss-cli

# Build
./scripts/hugo || exit 1

# Move html files to specific directory
sudo mv /home/admin/proxima/data/be-opensource/public /home/admin/proxima/data/be-opensource/public-temp
sudo mv public /home/admin/proxima/data/be-opensource/
sudo rm -rf /home/admin/proxima/data/be-opensource/public-temp

