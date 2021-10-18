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
##  \author   haichao.chc
##  \date     Oct 2020
##  \brief    Detail cmake build script
##

include (../../cmake/bazel.cmake)

git_version(PROXIMA_BE_CLIENT_VERSION ${CMAKE_CURRENT_SOURCE_DIR})

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

cc_library(
    NAME proxima_search_client STATIC SHARED STRICT PACKED
    SRCS src/*.cc
    LIBS proxima_be_proto
         proxima_be_common
         brpc
         proxima
         ${CMAKE_THREAD_LIBS_INIT}
         ${CMAKE_DL_LIBS}
    INCS . .. ./include/ ../../src/
    PUBINCS ./include/
    LDFLAGS ${APPLE_FRAMEWORK_LIBS}
    VERSION "${PROXIMA_BE_CLIENT_VERSION}"
  )

cc_binary(
    NAME client_example STRICT
    SRCS examples/client_example.cc
    LIBS proxima_search_client
    INCS . .. ./include/
  )

set_target_properties(client_example PROPERTIES INSTALL_RPATH ${LIB_PATH})
