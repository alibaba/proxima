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
##  \brief    CMakeLists.txt for tools

include(../cmake/bazel.cmake)
include(../cmake/option.cmake)

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

cc_binary(
    NAME bench_client PACKED
    SRCS bench_client.cc
    LIBS proxima_search_client
         proxima_be_common
         proxima
         brpc
         ${CMAKE_THREAD_LIBS_INIT}
         ${CMAKE_DL_LIBS}
    INCS ../src/ ../sdk/cpp/include/
    LDFLAGS ${APPLE_FRAMEWORK_LIBS}
  )
set_target_properties(bench_client PROPERTIES INSTALL_RPATH ${LIB_PATH})

cc_binary(
    NAME admin_client PACKED
    SRCS admin_client.cc
    LIBS proxima_be_proto
         proxima_be_common
         proxima
         brpc
         ${CMAKE_THREAD_LIBS_INIT}
         ${CMAKE_DL_LIBS}
    INCS ../src/
    LDFLAGS ${APPLE_FRAMEWORK_LIBS}
  )
set_target_properties(admin_client PROPERTIES INSTALL_RPATH ${LIB_PATH})

cc_binary(
  NAME index_builder PACKED
  SRCS index_builder.cc
  LIBS proxima_be_proto
       proxima_be_common
       proxima_be_index
       proxima
       brpc
       ${CMAKE_THREAD_LIBS_INIT}
       ${CMAKE_DL_LIBS}
  INCS ../src/
  LDFLAGS ${APPLE_FRAMEWORK_LIBS}
  )
set_target_properties(index_builder PROPERTIES INSTALL_RPATH ${LIB_PATH})
