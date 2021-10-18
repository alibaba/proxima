##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Mar 2018
##  \version  1.0
##  \brief    Detail cmake build script (thirdparty)
##

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)
project(thirdparty)

set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(EXTERNAL_BINARY_DIR ${CMAKE_BINARY_DIR}/external)

## EXCLUDE_FROM_ALL
add_subdirectory(openssl openssl EXCLUDE_FROM_ALL)
add_subdirectory(ncurses ncurses EXCLUDE_FROM_ALL)
add_subdirectory(googletest googletest EXCLUDE_FROM_ALL)
add_subdirectory(gflags gflags EXCLUDE_FROM_ALL)
add_subdirectory(glog glog EXCLUDE_FROM_ALL)
add_subdirectory(snappy snappy EXCLUDE_FROM_ALL)
add_subdirectory(leveldb leveldb EXCLUDE_FROM_ALL)
add_subdirectory(sparsehash sparsehash EXCLUDE_FROM_ALL)
add_subdirectory(protobuf protobuf EXCLUDE_FROM_ALL)
add_subdirectory(brpc brpc EXCLUDE_FROM_ALL)
add_subdirectory(mysql mysql EXCLUDE_FROM_ALL)
add_subdirectory(sqlite sqlite EXCLUDE_FROM_ALL)
