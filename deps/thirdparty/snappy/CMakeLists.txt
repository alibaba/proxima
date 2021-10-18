##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for leveldb (thirdparty)
##

set(SNAPPY_BUILD_TESTS OFF CACHE BOOL "Disable testing in snappy" FORCE)
set(SNAPPY_INSTALL OFF CACHE BOOL "Disable install in snappy" FORCE)

add_subdirectory(snappy)
