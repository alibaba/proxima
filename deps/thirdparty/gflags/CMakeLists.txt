##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for gflags (thirdparty)
##

set(BUILD_TESTING OFF CACHE BOOL "Disable Unit Tests" FORCE)
set(GFLAGS_BUILD_TESTING OFF CACHE BOOL "Disable unittest in gflags" FORCE)
if(NOT BUILD_SHARED_LIBS)
  set(GFLAGS_BUILD_SHARED_LIBS OFF)
  set(GFLAGS_BUILD_STATIC_LIBS ON)
else()
  set(GFLAGS_BUILD_SHARED_LIBS ON)
  set(GFLAGS_BUILD_STATIC_LIBS OFF)
endif()

set(GFLAGS_IS_SUBPROJECT ON)
add_subdirectory(gflags)

get_target_property(gflags_OUTPUT_NAME gflags OUTPUT_NAME)
get_target_property(gflags_BINARY_DIR gflags BINARY_DIR)
get_target_property(gflags_SOURCE_DIR gflags SOURCE_DIR)
get_target_property(gflags_PREFIX gflags PREFIX)
get_target_property(gflags_SUFFIX gflags SUFFIX)

set(gflags_OUTPUT_DIR ${gflags_BINARY_DIR})
if(NOT BUILD_SHARED_LIBS)
  if (NOT gflags_PREFIX)
    set(gflags_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  endif()
  if (NOT gflags_SUFFIX)
    set(gflags_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif()
  if(CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(gflags_OUTPUT_DIR ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  endif()
else()
  if (NOT gflags_PREFIX)
    set(gflags_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
  endif()
  if (NOT gflags_SUFFIX)
    set(gflags_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(gflags_OUTPUT_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
endif()

set(gflags_FOUND TRUE PARENT_SCOPE)
set(gflags_INCLUDE_DIR ${gflags_BINARY_DIR}/include PARENT_SCOPE)
set(gflags_LIBRARIES ${gflags_OUTPUT_DIR}/${gflags_PREFIX}${gflags_OUTPUT_NAME}${gflags_SUFFIX} PARENT_SCOPE)
set(gflags_DIR ${gflags_BINARY_DIR} PARENT_SCOPE)

set(GFLAGS_FOUND TRUE PARENT_SCOPE)
set(GFLAGS_INCLUDE_DIR ${gflags_BINARY_DIR}/include PARENT_SCOPE)
set(GFLAGS_LIBRARIES ${gflags_OUTPUT_DIR}/${gflags_PREFIX}${gflags_OUTPUT_NAME}${gflags_SUFFIX} PARENT_SCOPE)
set(GFLAGS_DIR ${gflags_BINARY_DIR} PARENT_SCOPE)
