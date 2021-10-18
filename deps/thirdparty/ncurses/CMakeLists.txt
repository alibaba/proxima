##
##  Copyright (C) The Software Authors. All rights reserved.
##
##  \file     CMakeLists.txt
##  \author   Hechong.xyf
##  \date     Nov 2020
##  \version  1.0
##  \brief    Detail cmake build script for ncurses (thirdparty)
##

include(ExternalProject)
ExternalProject_Add(
    Ncurses.BUILD
    PREFIX ncurses
    URL "https://ftp.gnu.org/gnu/ncurses/ncurses-6.2.tar.gz"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
    --without-manpages --without-progs --without-tack --without-tests --with-normal
    --disable-db-install --without-ada
    --with-build-cc=${CMAKE_C_COMPILER} --with-build-cpp=${CMAKE_CXX_COMPILER}
    --with-build-cflags=${CMAKE_C_CFLAGS} --with-build-cppflags=${CMAKE_CXX_CFLAGS}
    BUILD_COMMAND make -j all
    INSTALL_COMMAND make DESTDIR=${EXTERNAL_BINARY_DIR} install
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
  )

set(NCURSES_INCLUDE_DIR ${EXTERNAL_BINARY_DIR}/usr/include)
set(NCURSES_LIBRARY_DIR ${EXTERNAL_BINARY_DIR}/usr/lib)
set(NCURSES_LIBRARIES ${NCURSES_LIBRARY_DIR}/libcurses.a)

set(CURSES_FOUND TRUE PARENT_SCOPE)
set(CURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIR} PARENT_SCOPE)
set(CURSES_LIBRARIES ${NCURSES_LIBRARIES} PARENT_SCOPE)
set(CURSES_CFLAGS ${CMAKE_C_COMPILER} PARENT_SCOPE)
set(CURSES_HAVE_CURSES_H TRUE PARENT_SCOPE)
set(CURSES_HAVE_NCURSES_H TRUE PARENT_SCOPE)
set(CURSES_HAVE_NCURSES_NCURSES_H FALSE PARENT_SCOPE)
set(CURSES_HAVE_NCURSES_CURSES_H FALSE PARENT_SCOPE)
set(CURSES_INCLUDE_DIR ${NCURSES_INCLUDE_DIR} PARENT_SCOPE)
set(CURSES_LIBRARY ${NCURSES_LIBRARIES} PARENT_SCOPE)
set(CURSES_INCLUDE_PATH ${NCURSES_INCLUDE_DIR} PARENT_SCOPE)
