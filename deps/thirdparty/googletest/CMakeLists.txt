##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for glog (thirdparty)
##

add_subdirectory(googletest)
get_target_property(GTEST_INCLUDE_DIRS gtest INTERFACE_INCLUDE_DIRECTORIES)

set(GTEST_FOUND TRUE PARENT_SCOPE)
set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} PARENT_SCOPE)
set(GTEST_LIBRARIES $<TARGET_FILE:gtest> PARENT_SCOPE)
set(GTEST_MAIN_LIBRARIES $<TARGET_FILE:gtest_main> PARENT_SCOPE)
set(GTEST_BOTH_LIBRARIES $<TARGET_FILE:gtest> $<TARGET_FILE:gtest_main> PARENT_SCOPE)
