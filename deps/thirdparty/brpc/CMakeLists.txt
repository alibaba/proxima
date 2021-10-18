##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for brpc (thirdparty)
##

set(GFLAGS_INCLUDE_PATH ${GFLAGS_INCLUDE_DIR})
set(GFLAGS_LIBRARY ${GFLAGS_LIBRARIES})

set(GLOG_INCLUDE_PATH ${GLOG_INCLUDE_DIR})
set(GLOG_LIB ${GLOG_LIBRARY})

set(LEVELDB_INCLUDE_PATH ${LEVELDB_INCLUDE_DIR})
set(LEVELDB_LIB ${LEVELDB_LIBRARY})

set(PROTOC_LIB ${PROTOBUF_PROTOC_LIBRARY})

set(DOWNLOAD_GTEST OFF CACHE BOOL "Disable download gtest in leveldb" FORCE)
set(WITH_GLOG ON CACHE BOOL "Enable glog in brpc" FORCE)

set(BRPC_FOLDER brpc)

add_compile_options(-Wno-shadow -Wno-format -Wno-deprecated-declarations -Wno-sign-compare -Wno-pessimizing-move -Wno-mismatched-tags -Wno-unused-private-field -Wno-unused-local-typedefs -Wno-delete-abstract-non-virtual-dtor -Wno-inconsistent-missing-override)

add_subdirectory(${BRPC_FOLDER})
add_dependencies(BUTIL_LIB OpenSSL.SSL OpenSSL.Crypto protoc glog leveldb gflags)
add_dependencies(SOURCES_LIB OpenSSL.SSL OpenSSL.Crypto protoc glog leveldb gflags)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${BRPC_FOLDER}/output/include)
add_library(brpc UNKNOWN IMPORTED GLOBAL)
set_target_properties(
    brpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/${BRPC_FOLDER}/output/include"
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/${BRPC_FOLDER}/output/lib/libbrpc.a"
    INTERFACE_LINK_LIBRARIES "leveldb;libprotobuf;glog;gflags;OpenSSL.SSL;OpenSSL.Crypto"
  )
add_dependencies(brpc brpc-static)
