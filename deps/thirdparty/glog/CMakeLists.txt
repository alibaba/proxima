##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for googletest (thirdparty)
##

set(BUILD_TESTING OFF CACHE BOOL "Disable Unit Tests" FORCE)
set(WITH_GFLAGS OFF CACHE BOOL "Disable find_package(gflags) in glog" FORCE)
set(HAVE_LIB_GFLAGS TRUE CACHE BOOL "")

add_compile_options(-Wno-deprecated-declarations)

add_subdirectory(glog)
add_dependencies(glog gflags)

get_target_property(glog_BINARY_DIR glog BINARY_DIR)
get_target_property(glog_SOURCE_DIR glog SOURCE_DIR)
# get_target_property(GLOG_INCLUDE_DIRS glog INTERFACE_INCLUDE_DIRECTORIES)

set(GLOG_INCLUDE_DIRS ${glog_BINARY_DIR} ${glog_SOURCE_DIR}/src)
set(GLOG_FOUND TRUE PARENT_SCOPE)
set(GLOG_INCLUDE_DIR ${GLOG_INCLUDE_DIRS} PARENT_SCOPE)
set(GLOG_INCLUDE_DIRS ${GLOG_INCLUDE_DIRS} PARENT_SCOPE)
set(GLOG_LIBRARY $<TARGET_FILE:glog> PARENT_SCOPE)
set(GLOG_LIBRARIES $<TARGET_FILE:glog> PARENT_SCOPE)
