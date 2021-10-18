##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for protobuf (thirdparty)
##

set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable testing in protobuf" FORCE)
set(protobuf_WITH_ZLIB ON CACHE BOOL "Disable zlib support in protobuf" FORCE)
add_subdirectory(protobuf/cmake protobuf)

get_target_property(libprotobuf_SOURCE_DIR libprotobuf SOURCE_DIR)
get_filename_component(libprotobuf_INCLUDE_DIR ${libprotobuf_SOURCE_DIR}/../src ABSOLUTE)

# set(Protobuf_FOUND TRUE PARENT_SCOPE)
# set(Protobuf_INCLUDE_DIR ${libprotobuf_INCLUDE_DIR} PARENT_SCOPE)
# set(Protobuf_INCLUDE_DIRS ${libprotobuf_INCLUDE_DIR} PARENT_SCOPE)

# set(Protobuf_LIBRARY $<TARGET_FILE:libprotobuf> PARENT_SCOPE)
# set(Protobuf_LIBRARIES $<TARGET_FILE:libprotobuf> PARENT_SCOPE)

# set(Protobuf_LITE_LIBRARY $<TARGET_FILE:libprotobuf-lite> PARENT_SCOPE)
# set(Protobuf_LITE_LIBRARIES $<TARGET_FILE:libprotobuf-lite> PARENT_SCOPE)

# set(Protobuf_PROTOC_LIBRARY $<TARGET_FILE:libprotoc> PARENT_SCOPE)
# set(Protobuf_PROTOC_LIBRARIES $<TARGET_FILE:libprotoc> PARENT_SCOPE)
# set(Protobuf_PROTOC_EXECUTABLE $<TARGET_FILE:protoc> PARENT_SCOPE)

set(PROTOBUF_FOUND TRUE PARENT_SCOPE)
set(PROTOBUF_INCLUDE_DIR ${libprotobuf_INCLUDE_DIR} PARENT_SCOPE)
set(PROTOBUF_INCLUDE_DIRS ${libprotobuf_INCLUDE_DIR} PARENT_SCOPE)

set(PROTOBUF_LIBRARY $<TARGET_FILE:libprotobuf> PARENT_SCOPE)
set(PROTOBUF_LIBRARIES $<TARGET_FILE:libprotobuf> PARENT_SCOPE)

set(PROTOBUF_LITE_LIBRARY $<TARGET_FILE:libprotobuf-lite> PARENT_SCOPE)
set(PROTOBUF_LITE_LIBRARIES $<TARGET_FILE:libprotobuf-lite> PARENT_SCOPE)

set(PROTOBUF_PROTOC_LIBRARY $<TARGET_FILE:libprotoc> PARENT_SCOPE)
set(PROTOBUF_PROTOC_LIBRARIES $<TARGET_FILE:libprotoc> PARENT_SCOPE)
set(PROTOBUF_PROTOC_EXECUTABLE $<TARGET_FILE:protoc> PARENT_SCOPE)

