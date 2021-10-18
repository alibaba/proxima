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

include(../cmake/bazel.cmake)
include(../cmake/option.cmake)

cc_proto_library(
    NAME proxima_be_proto STATIC
    SRCS proto/*.proto
    PROTOROOT ./
  )

# Retrieve version from git repository
git_version(PROXIMA_BE_VERSION ${CMAKE_CURRENT_SOURCE_DIR})

# Add repository
cc_directory(common)
cc_directory(metrics)
cc_directory(meta)
cc_directory(index)
cc_directory(query)
cc_directory(agent)
cc_directory(admin)
cc_directory(server)
cc_directory(repository)
