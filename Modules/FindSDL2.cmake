#.rst:
# FindSDL2
# ---------
#
# Find the Simple DirectMedia Layer (version 2.x.x).
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module sets the following variables:
#
# ``SDL2_FOUND``
#  True, if the system has SDL2.
# ``SDL2_INCLUDE_DIR``
#  Path to the SDL2 include directory.
# ``SDL2_LIBRARIES``
#  Paths to the SDL2 libraries.
# ``SDL2_LINK_FLAGS``
#  Additional linker flags for SDL2.
# ``SDL2_SHARED_LIBRARY_PATH``
#  Paths to the SDL2 shared library.


#=============================================================================
# Copyright 2015 Jacek Migacz (jacekmigacz@gmail.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (WIN32)
    if (DEFINED ENV{SDL2_ROOT_DIR})
        if (MINGW)
            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                find_path(SDL2_INCLUDE_DIR SDL.h HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES i686-w64-mingw32/include/SDL2 DOC "SDL2 include dir.")
                find_library(SDL2_sdl2_LIBRARY SDL2 HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES i686-w64-mingw32/lib)
                find_library(SDL2_main_LIBRARY SDL2main HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES i686-w64-mingw32/lib)
                find_file(SDL2_SHARED_LIBRARY_PATH SDL2.dll HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES i686-w64-mingw32/bin DOC "SDL2 shared library path.")
            elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
                find_path(SDL2_INCLUDE_DIR SDL.h HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES x86_64-w64-mingw32/include/SDL2 DOC "SDL2 include dir.")
                find_library(SDL2_sdl2_LIBRARY SDL2 HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES x86_64-w64-mingw32/lib)
                find_library(SDL2_main_LIBRARY SDL2main HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES x86_64-w64-mingw32/lib)
                find_file(SDL2_SHARED_LIBRARY_PATH SDL2.dll HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES x86_64-w64-mingw32/bin DOC "SDL2 shared library path.")
            endif ()
        elseif (MSVC)
            find_path(SDL2_INCLUDE_DIR SDL.h HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES include DOC "SDL2 include dir.")
            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                find_library(SDL2_sdl2_LIBRARY SDL2 HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x86)
                find_library(SDL2_main_LIBRARY SDL2main HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x86)
                find_file(SDL2_SHARED_LIBRARY_PATH SDL2.dll HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x86 DOC "SDL2 shared library path.")
            elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
                find_library(SDL2_sdl2_LIBRARY SDL2 HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x64)
                find_library(SDL2_main_LIBRARY SDL2main HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x64)
                find_file(SDL2_SHARED_LIBRARY_PATH SDL2.dll HINTS $ENV{SDL2_ROOT_DIR} PATH_SUFFIXES lib/x64 DOC "SDL2 shared library path.")
            endif ()
        endif ()
        set(SDL2_LIBRARIES ${SDL2_main_LIBRARY} ${SDL2_sdl2_LIBRARY} CACHE STRING "SDL2 libraries.")
        unset(SDL2_main_LIBRARY CACHE)
        unset(SDL2_sdl2_LIBRARY CACHE)
        if (MINGW)
            set(SDL2_LIBRARIES mingw32 ${SDL2_LIBRARIES})
            set(SDL2_LINK_FLAGS "-mwindows" CACHE STRING "SDL2 additional linker flags.")
        elseif (MSVC)
            set(SDL2_LINK_FLAGS "/SUBSYSTEM:WINDOWS" CACHE STRING "SDL2 additional linker flags.")	
        endif ()
    else ()
        message(WARNING "SDL2_ROOT_DIR is not set.")
    endif ()
endif ()

if (SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL_VERSION_STRING REGEX "^#define SDL_([A-Z]*(_VERSION|PATCHLEVEL))[ ]+[0-9]+")
    string(REGEX REPLACE ".*#define SDL_MAJOR_VERSION[ ]+([0-9]+).*" "\\1" SDL_MAJOR_VERSION ${SDL_VERSION_STRING})
    string(REGEX REPLACE ".*#define SDL_MINOR_VERSION[ ]+([0-9]+).*" "\\1" SDL_MINOR_VERSION ${SDL_VERSION_STRING})
    string(REGEX REPLACE ".*#define SDL_PATCHLEVEL[ ]+([0-9]+).*" "\\1" SDL_PATCHLEVEL ${SDL_VERSION_STRING})
    set(SDL2_VERSION_STRING ${SDL_MAJOR_VERSION}.${SDL_MINOR_VERSION}.${SDL_PATCHLEVEL})
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARIES VERSION_VAR SDL2_VERSION_STRING)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARIES SDL2_LINK_FLAGS SDL2_SHARED_LIBRARY_PATH)
