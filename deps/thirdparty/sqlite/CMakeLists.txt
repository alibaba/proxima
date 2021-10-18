##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Mar 2018
##  \version  1.0
##  \brief    Detail cmake build script (sqlite)
##

cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)
project(sqlite)

# sqlite source folder
http_archive(
    NAME sqlite
    URL https://sqlite.org/2021/sqlite-amalgamation-3360000.zip
    PATH sqlite-3.36.0
)
set(SQLITE_FOLDER sqlite-3.36.0)

find_package(Threads REQUIRED)

add_library(sqlite STATIC ${SQLITE_FOLDER}/sqlite3.c)
target_link_libraries(sqlite ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
target_include_directories(sqlite PUBLIC ${SQLITE_FOLDER})

add_executable(sqlite_shell ${SQLITE_FOLDER}/shell.c)
target_link_libraries(sqlite_shell sqlite)
