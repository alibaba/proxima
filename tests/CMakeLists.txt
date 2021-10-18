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
##  \date     Feb 2021
##  \brief    Detail cmake build script of unittest
##

include(../cmake/bazel.cmake)
include(../cmake/option.cmake)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/integration ${CMAKE_CURRENT_BINARY_DIR}/integration
  )

cc_directories(common meta index agent query server admin)
cc_directory(repository)
