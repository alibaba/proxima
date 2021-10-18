##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   xiaoxin.gxx
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script
##

cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)
project(proxima-be-project)

include(cmake/bazel.cmake)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Install benchmark tools
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark ${CMAKE_CURRENT_BINARY_DIR}/benchmark
  )

# Install git hooks
execute_process(
    COMMAND sh scripts/install-git-hooks.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

cc_directory(deps/thirdparty)
cc_directory(deps/proxima/)
get_property(PROXIMA_LIB GLOBAL PROPERTY proxima_lib_property)
set(LIB_PATH "${PROXIMA_LIB}:$ORIGIN/../lib64:$ORIGIN/../lib:$ORIGIN")

cc_directories(src tests sdk tools)

if (ENABLE_LOCAL_INTEGRATION)
  add_custom_command(OUTPUT integration_test
    COMMAND bash tests/integration/run.sh local
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running integration tests"
    VERBATIM
  )
else()
  add_custom_command(OUTPUT integration_test
    COMMAND bash tests/integration/run.sh docker
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Running integration tests"
    VERBATIM
  ) 
endif()
add_custom_target(integration DEPENDS integration_test)

# install conf files
install(FILES scripts/conf/proxima_be.conf DESTINATION ${CMAKE_INSTALL_BINDIR}/../conf)

git_version(GIT_SRCS_VER ${CMAKE_CURRENT_SOURCE_DIR})
set(CPACK_PACKAGE_VERSION ${GIT_SRCS_VER})
set(CPACK_PACKAGE_NAME proxima-be)
include(CPack)
