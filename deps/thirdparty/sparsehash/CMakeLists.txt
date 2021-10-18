##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for glog (thirdparty)
##

add_library(sparsehash INTERFACE)
target_include_directories(
    sparsehash INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/sparsehash/src/"
  )

