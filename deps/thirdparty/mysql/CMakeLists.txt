##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for mysql (thirdparty)
##

include(ExternalProject)

set(MYSQL_INCLUDE_DIR "${EXTERNAL_BINARY_DIR}/usr/local/mysql/include")
set(MYSQL_LIBRARY_DIR "${EXTERNAL_BINARY_DIR}/usr/local/mysql/lib/mysql")
file(MAKE_DIRECTORY ${MYSQL_INCLUDE_DIR})
file(MAKE_DIRECTORY ${MYSQL_LIBRARY_DIR})

## https://dev.mysql.com/doc/refman/5.7/en/installing-source-distribution.html
list(APPEND CMAKE_CACHE_ARGS -DWITHOUT_SERVER:BOOL=OFF)
list(APPEND CMAKE_CACHE_ARGS -DWITH_EMBEDDED_SERVER:BOOL=ON)
list(APPEND CMAKE_CACHE_ARGS -DWITH_DEFAULT_FEATURE_SET:BOOL=OFF)
list(APPEND CMAKE_CACHE_ARGS -DWITH_SSL:PATH=${OPENSSL_INCLUDE_DIR}/..)
list(APPEND CMAKE_CACHE_ARGS -DWITH_DEBUG:BOOL=OFF)
list(APPEND CMAKE_CACHE_ARGS -DWITH_LIBEVENT:STRING=bundled)
list(APPEND CMAKE_CACHE_ARGS -DWITH_UNIT_TESTS:BOOL=OFF)
list(APPEND CMAKE_CACHE_ARGS -DBOOST_INCLUDE_DIR:PATH=<SOURCE_DIR>/boost/boost_1_59_0)

list(APPEND CMAKE_CACHE_ARGS -DCURSES_LIBRARY:PATH=${CURSES_LIBRARY})
list(APPEND CMAKE_CACHE_ARGS -DCURSES_INCLUDE_PATH:PATH=${CURSES_INCLUDE_PATH})

ExternalProject_Add(
    MySQL.BUILD PREFIX mysql
    URL "http://ftp.ntu.edu.tw/MySQL/Downloads/MySQL-5.7/mysql-boost-5.7.33.tar.gz"
    CONFIGURE_COMMAND "${CMAKE_COMMAND}" ${CMAKE_CACHE_ARGS} -G ${CMAKE_GENERATOR} "<SOURCE_DIR>"
    BUILD_COMMAND "${CMAKE_COMMAND}" --build . --target mysqlclient -- -j
    INSTALL_COMMAND "${CMAKE_COMMAND}" -E env DESTDIR=${EXTERNAL_BINARY_DIR} "${CMAKE_COMMAND}" --build ./include --target install
    COMMAND "${CMAKE_COMMAND}" -E copy "<SOURCE_DIR>/include/hash.h" "${MYSQL_INCLUDE_DIR}/"
    COMMAND "${CMAKE_COMMAND}" -E copy "<SOURCE_DIR>/include/myisampack.h" "${MYSQL_INCLUDE_DIR}/"
    COMMAND "${CMAKE_COMMAND}" -E copy ./archive_output_directory/libmysqlclient.a "${MYSQL_LIBRARY_DIR}/"
    COMMAND "${CMAKE_COMMAND}" -E copy ./archive_output_directory/libstrings.a "${MYSQL_LIBRARY_DIR}/"
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
  )
add_dependencies(MySQL.BUILD OpenSSL.BUILD)
add_dependencies(MySQL.BUILD Ncurses.BUILD)

add_library(MySQL.mysqlclient UNKNOWN IMPORTED GLOBAL)
set_target_properties(
    MySQL.mysqlclient PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
    IMPORTED_LOCATION "${MYSQL_LIBRARY_DIR}/libmysqlclient.a"
  )
add_dependencies(MySQL.mysqlclient MySQL.BUILD)

add_library(MySQL.strings UNKNOWN IMPORTED GLOBAL)
set_target_properties(
    MySQL.strings PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
    IMPORTED_LOCATION "${MYSQL_LIBRARY_DIR}/libstrings.a"
  )
add_dependencies(MySQL.strings MySQL.BUILD)
