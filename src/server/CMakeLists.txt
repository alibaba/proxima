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
##  \author   hechong.xyf
##  \date     Oct 2020
##  \brief    Detail cmake build script
##

include(../../cmake/bazel.cmake)
include(../../cmake/option.cmake)

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

# proxima_be_agent library
aux_source_directory(. SERVER_SRCS)
list(REMOVE_ITEM SERVER_SRCS "./main.cc")
cc_library(
    NAME proxima_be_server STATIC STRICT
    SRCS ${SERVER_SRCS}
    LIBS proxima_be_admin
         proxima_be_agent
         proxima_be_query
         proxima_be_index
         proxima_be_meta
         proxima_be_common
         proxima_be_proto
         proxima_be_metrics
    INCS . ..
    VERSION "${PROXIMA_BE_VERSION}"
  )

# proxima_be binary
cc_binary(
    NAME proxima_be PACKED
    SRCS main.cc
    LIBS proxima_be_server
         proxima_be_admin
         proxima_be_agent
         proxima_be_query
         proxima_be_index
         proxima_be_meta
         proxima_be_common
         proxima_be_proto
         proxima_be_metrics
         brpc
         ${CMAKE_THREAD_LIBS_INIT}
         ${CMAKE_DL_LIBS}
    DEPS proxima
    INCS ..
    LDFLAGS ${APPLE_FRAMEWORK_LIBS}
  )

set_target_properties(proxima_be PROPERTIES INSTALL_RPATH ${LIB_PATH})
