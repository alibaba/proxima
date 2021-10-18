##
##  Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##  
##      http://www.apache.org/licenses/LICENSE-2.0
##  
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
##  \author   Dianzhang.Chen
##  \date     Dec 2020
##  \brief    Detail cmake build script for repository
##

include(../../cmake/bazel.cmake)
include(../../cmake/option.cmake)

# Retrieve version from git repository
git_version(MYSQL_REPOSITORY_VERSION ${CMAKE_CURRENT_SOURCE_DIR})

cc_directory(binlog)
cc_directory(repository_common)

if(APPLE)
  set(APPLE_FRAMEWORK_LIBS
      -framework CoreFoundation
      -framework CoreGraphics
      -framework CoreData
      -framework CoreText
      -framework Security
      -framework Foundation
      -Wl,-U,_MallocExtension_ReleaseFreeMemory
      -Wl,-U,_ProfilerStart
      -Wl,-U,_ProfilerStop
      -Wl,-U,_RegisterThriftProtocol
    )
endif()

cc_proto_library(
    NAME mysql_repository_proto STATIC
    SRCS proto/*.proto
    PROTOROOT ./
  )

# proxima_be_repo library
cc_library(
    NAME mysql_repository_lib STATIC STRICT
    SRCS collection*.cc mysql_collection*.cc lsn_context_*.cc
    LIBS mysql_binlog
         proxima_be_proto
         mysql_repository_common
         brpc
         ${CMAKE_DL_LIBS}
    INCS . ..
    VERSION "${MYSQL_REPOSITORY_VERSION}"
  )

# mysql repository binary
cc_binary(
    NAME mysql_repository PACKED
    SRCS main.cc mysql_repository.cc
    LIBS OpenSSL.SSL
         OpenSSL.Crypto
         mysql_repository_lib
         mysql_repository_common
         proxima_be_proto
         ${CMAKE_THREAD_LIBS_INIT}
    INCS . ..
    LDFLAGS ${APPLE_FRAMEWORK_LIBS}
  )
set_target_properties(mysql_repository PROPERTIES INSTALL_RPATH ${LIB_PATH})
