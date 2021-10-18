##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for leveldb (thirdparty)
##

set(HAVE_TCMALLOC OFF CACHE BOOL "Disable tcmalloc in leveldb" FORCE)
set(HAVE_SNAPPY ON CACHE BOOL "Enable snappy in leveldb" FORCE)
set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "Disable testing in leveldb" FORCE)
set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "Disable benchmarks in leveldb" FORCE)
set(LEVELDB_INSTALL OFF CACHE BOOL "Disable install in leveldb" FORCE)

add_subdirectory(leveldb)

get_target_property(leveldb_SOURCE_DIR leveldb SOURCE_DIR)
set(LEVELDB_FOUND TRUE PARENT_SCOPE)
set(LEVELDB_INCLUDE_DIR ${leveldb_SOURCE_DIR}/include PARENT_SCOPE)
set(LEVELDB_INCLUDE_DIRS ${leveldb_SOURCE_DIR}/include PARENT_SCOPE)
set(LEVELDB_LIBRARY $<TARGET_FILE:leveldb> PARENT_SCOPE)
set(LEVELDB_LIBRARIES $<TARGET_FILE:leveldb> PARENT_SCOPE)
