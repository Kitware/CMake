
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

# This module is used by the Makefile generator to determin the following variables:
# CMAKE_SYSTEM_NAME - on unix this is uname -s, for windows it is Windows
# CMAKE_SYSTEM_VERSION - on unix this is uname -r, for windows it is empty
# CMAKE_SYSTEM - ${CMAKE_SYSTEM}-${CMAKE_SYSTEM_VERSION}, for windows: ${CMAKE_SYSTEM}
#
#  Expected uname -s output:
#
# AIX                           AIX
# BSD/OS                        BSD/OS
# FreeBSD                       FreeBSD
# HP-UX                         HP-UX
# IRIX                          IRIX
# Linux                         Linux
# GNU/kFreeBSD                  GNU/kFreeBSD
# NetBSD                        NetBSD
# OpenBSD                       OpenBSD
# OFS/1 (Digital Unix)          OSF1
# SCO OpenServer 5              SCO_SV
# SCO UnixWare 7                UnixWare
# SCO UnixWare (pre release 7)  UNIX_SV
# SCO XENIX                     Xenix
# Solaris                       SunOS
# SunOS                         SunOS
# Tru64                         Tru64
# Ultrix                        ULTRIX
# cygwin                        CYGWIN_NT-5.1
# MacOSX                        Darwin


# find out on which system cmake runs
if(CMAKE_HOST_UNIX)
  find_program(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
  if(CMAKE_UNAME)
    exec_program(uname ARGS -s OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_NAME)
    exec_program(uname ARGS -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION)
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux|CYGWIN.*")
      exec_program(uname ARGS -m OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_PROCESSOR
        RETURN_VALUE val)
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "OpenBSD")
      exec_program(arch ARGS -s OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_PROCESSOR
        RETURN_VALUE val)
    else()
      exec_program(uname ARGS -p OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_PROCESSOR
        RETURN_VALUE val)
      if("${val}" GREATER 0)
        exec_program(uname ARGS -m OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_PROCESSOR
          RETURN_VALUE val)
      endif("${val}" GREATER 0)
    endif()
    # check the return of the last uname -m or -p
    if("${val}" GREATER 0)
        set(CMAKE_HOST_SYSTEM_PROCESSOR "unknown")
    endif("${val}" GREATER 0)
    set(CMAKE_UNAME ${CMAKE_UNAME} CACHE INTERNAL "uname command")
    # processor may have double quote in the name, and that needs to be removed
    string(REGEX REPLACE "\"" "" CMAKE_HOST_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    string(REGEX REPLACE "/" "_" CMAKE_HOST_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
  endif(CMAKE_UNAME)
else(CMAKE_HOST_UNIX)
  if(CMAKE_HOST_WIN32)
    set(CMAKE_HOST_SYSTEM_NAME "Windows")
    set(CMAKE_HOST_SYSTEM_PROCESSOR "$ENV{PROCESSOR_ARCHITECTURE}")
  endif(CMAKE_HOST_WIN32)
endif(CMAKE_HOST_UNIX)

# if a toolchain file is used, the user wants to cross compile.
# in this case read the toolchain file and keep the CMAKE_HOST_SYSTEM_*
# variables around so they can be used in CMakeLists.txt.
# In all other cases, the host and target platform are the same.
if(CMAKE_TOOLCHAIN_FILE)
  # at first try to load it as path relative to the directory from which cmake has been run
  include("${CMAKE_BINARY_DIR}/${CMAKE_TOOLCHAIN_FILE}" OPTIONAL RESULT_VARIABLE _INCLUDED_TOOLCHAIN_FILE)
  if(NOT _INCLUDED_TOOLCHAIN_FILE)
     # if the file isn't found there, check the default locations
     include("${CMAKE_TOOLCHAIN_FILE}" OPTIONAL RESULT_VARIABLE _INCLUDED_TOOLCHAIN_FILE)
  endif(NOT _INCLUDED_TOOLCHAIN_FILE)

  if(_INCLUDED_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${_INCLUDED_TOOLCHAIN_FILE}" CACHE FILEPATH "The CMake toolchain file" FORCE)
  else(_INCLUDED_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Could not find toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    set(CMAKE_TOOLCHAIN_FILE "NOTFOUND" CACHE FILEPATH "The CMake toolchain file" FORCE)
  endif(_INCLUDED_TOOLCHAIN_FILE)
endif(CMAKE_TOOLCHAIN_FILE)


# if CMAKE_SYSTEM_NAME is here already set, either it comes from a toolchain file
# or it was set via -DCMAKE_SYSTEM_NAME=...
# if that's the case, assume we are crosscompiling
if(CMAKE_SYSTEM_NAME)
  if(NOT DEFINED CMAKE_CROSSCOMPILING)
    set(CMAKE_CROSSCOMPILING TRUE)
  endif(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(PRESET_CMAKE_SYSTEM_NAME TRUE)
else(CMAKE_SYSTEM_NAME)
  set(CMAKE_SYSTEM_NAME      "${CMAKE_HOST_SYSTEM_NAME}")
  set(CMAKE_SYSTEM_VERSION   "${CMAKE_HOST_SYSTEM_VERSION}")
  set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
  set(CMAKE_CROSSCOMPILING FALSE)
  set(PRESET_CMAKE_SYSTEM_NAME FALSE)
endif(CMAKE_SYSTEM_NAME)


macro(ADJUST_CMAKE_SYSTEM_VARIABLES _PREFIX)
  if(NOT ${_PREFIX}_NAME)
    set(${_PREFIX}_NAME "UnknownOS")
  endif(NOT ${_PREFIX}_NAME)

  # fix for BSD/OS , remove the /
  if(${_PREFIX}_NAME MATCHES BSD.OS)
    set(${_PREFIX}_NAME BSDOS)
  endif(${_PREFIX}_NAME MATCHES BSD.OS)

  # fix for GNU/kFreeBSD, remove the GNU/
  if(${_PREFIX}_NAME MATCHES kFreeBSD)
    set(${_PREFIX}_NAME kFreeBSD)
  endif(${_PREFIX}_NAME MATCHES kFreeBSD)

  # fix for CYGWIN which has windows version in it
  if(${_PREFIX}_NAME MATCHES CYGWIN)
    set(${_PREFIX}_NAME CYGWIN)
  endif(${_PREFIX}_NAME MATCHES CYGWIN)

  # set CMAKE_SYSTEM to the CMAKE_SYSTEM_NAME
  set(${_PREFIX}  ${${_PREFIX}_NAME})
  # if there is a CMAKE_SYSTEM_VERSION then add a -${CMAKE_SYSTEM_VERSION}
  if(${_PREFIX}_VERSION)
    set(${_PREFIX} ${${_PREFIX}}-${${_PREFIX}_VERSION})
  endif(${_PREFIX}_VERSION)

endmacro(ADJUST_CMAKE_SYSTEM_VARIABLES _PREFIX)

ADJUST_CMAKE_SYSTEM_VARIABLES(CMAKE_SYSTEM)
ADJUST_CMAKE_SYSTEM_VARIABLES(CMAKE_HOST_SYSTEM)

# this file is also executed from cpack, then we don't need to generate these files
# in this case there is no CMAKE_BINARY_DIR
if(CMAKE_BINARY_DIR)
  # write entry to the log file
  if(PRESET_CMAKE_SYSTEM_NAME)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
                "The target system is: ${CMAKE_SYSTEM_NAME} - ${CMAKE_SYSTEM_VERSION} - ${CMAKE_SYSTEM_PROCESSOR}\n")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
                "The host system is: ${CMAKE_HOST_SYSTEM_NAME} - ${CMAKE_HOST_SYSTEM_VERSION} - ${CMAKE_HOST_SYSTEM_PROCESSOR}\n")
  else(PRESET_CMAKE_SYSTEM_NAME)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
                "The system is: ${CMAKE_SYSTEM_NAME} - ${CMAKE_SYSTEM_VERSION} - ${CMAKE_SYSTEM_PROCESSOR}\n")
  endif(PRESET_CMAKE_SYSTEM_NAME)

  # if a toolchain file is used, it needs to be included in the configured file,
  # so settings done there are also available if they don't go in the cache and in try_compile()
  set(INCLUDE_CMAKE_TOOLCHAIN_FILE_IF_REQUIRED)
  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    set(INCLUDE_CMAKE_TOOLCHAIN_FILE_IF_REQUIRED "include(\"${CMAKE_TOOLCHAIN_FILE}\")")
  endif(DEFINED CMAKE_TOOLCHAIN_FILE)

  # configure variables set in this file for fast reload, the template file is defined at the top of this file
  configure_file(${CMAKE_ROOT}/Modules/CMakeSystem.cmake.in
                ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeSystem.cmake
                IMMEDIATE @ONLY)

endif(CMAKE_BINARY_DIR)
