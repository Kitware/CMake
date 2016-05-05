# This file is copyrighted under the BSD-license for buildsystem files of KDE
# copyright 2010, Patrick Spendrin <ps_ml@gmx.de>

project(expat)

cmake_minimum_required(VERSION 2.6)

include(ConfigureChecks.cmake)

include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/lib)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -wd4996)
endif(MSVC)
if(WIN32)
    add_definitions(-DCOMPILING_FOR_WINDOWS)
endif(WIN32)

set(expat_SRCS
    lib/xmlparse.c
    lib/xmlrole.c
    lib/xmltok.c 
    lib/xmltok_impl.c 
    lib/xmltok_ns.c
)

if(BUILD_shared)
    set(_SHARED SHARED)
    if(WIN32)
        set(expat_SRCS ${expat_SRCS} lib/libexpat.def)
    endif(WIN32)
else(BUILD_shared)
    set(_SHARED STATIC)
    if(WIN32)
        add_definitions(-DXML_STATIC)
    endif(WIN32)
endif(BUILD_shared)

add_library(expat ${_SHARED} ${expat_SRCS})
