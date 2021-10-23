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
##  \author   daibing.db
##  \date     Aug 2021
##  \brief    Detail cmake build script
##

include(../../cmake/bazel.cmake)

set(ARCH "nehalem")
if(ENABLE_SKYLAKE_AVX512)
  set(ARCH "skylake_avx512")
elseif(ENABLE_SKYLAKE)
  set(ARCH "skylake")
elseif(ENABLE_BROADWELL)
  set(ARCH "broadwell")
elseif(ENABLE_HASWELL)
  set(ARCH "haswell")
elseif(ENABLE_SANDYBRIDGE)
  set(ARCH "sandybridge")
elseif(ENABLE_ZEN2)
  set(ARCH "znver2")
elseif(ENABLE_ZEN1)
  set(ARCH "znver1")
endif()

set(PROXIMA_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${ARCH}/")
if(NOT EXISTS ${PROXIMA_LIB})
  message(FATAL_ERROR "Not supported for ${CMAKE_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${ARCH}")
endif()
set_property(GLOBAL PROPERTY proxima_lib_property "${PROXIMA_LIB}")

cc_import(
    NAME proxima
    SHARED
    PATH ${PROXIMA_LIB}/libproxima.so
    PUBINCS include/
  )
install(DIRECTORY ${PROXIMA_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
