
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

# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-g++, arm-elf-ar etc.)
#
# Sets the following variables:
#   CMAKE_CXX_COMPILER
#   CMAKE_COMPILER_IS_GNUCXX
#   CMAKE_AR
#   CMAKE_RANLIB
#
# If not already set before, it also sets
#   _CMAKE_TOOLCHAIN_PREFIX

if(NOT CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER_INIT NOTFOUND)

  # prefer the environment variable CXX
  if($ENV{CXX} MATCHES ".+")
    get_filename_component(CMAKE_CXX_COMPILER_INIT $ENV{CXX} PROGRAM PROGRAM_ARGS CMAKE_CXX_FLAGS_ENV_INIT)
    if(CMAKE_CXX_FLAGS_ENV_INIT)
      set(CMAKE_CXX_COMPILER_ARG1 "${CMAKE_CXX_FLAGS_ENV_INIT}" CACHE STRING "First argument to CXX compiler")
    endif(CMAKE_CXX_FLAGS_ENV_INIT)
    if(NOT EXISTS ${CMAKE_CXX_COMPILER_INIT})
      message(FATAL_ERROR "Could not find compiler set in environment variable CXX:\n$ENV{CXX}.\n${CMAKE_CXX_COMPILER_INIT}")
    endif(NOT EXISTS ${CMAKE_CXX_COMPILER_INIT})
  endif($ENV{CXX} MATCHES ".+")

  # next prefer the generator specified compiler
  if(CMAKE_GENERATOR_CXX)
    if(NOT CMAKE_CXX_COMPILER_INIT)
      set(CMAKE_CXX_COMPILER_INIT ${CMAKE_GENERATOR_CXX})
    endif(NOT CMAKE_CXX_COMPILER_INIT)
  endif(CMAKE_GENERATOR_CXX)

  # finally list compilers to try
  if(CMAKE_CXX_COMPILER_INIT)
    set(CMAKE_CXX_COMPILER_LIST ${CMAKE_CXX_COMPILER_INIT})
  else(CMAKE_CXX_COMPILER_INIT)
    set(CMAKE_CXX_COMPILER_LIST ${_CMAKE_TOOLCHAIN_PREFIX}c++ ${_CMAKE_TOOLCHAIN_PREFIX}g++ CC aCC cl bcc xlC)
  endif(CMAKE_CXX_COMPILER_INIT)

  # Find the compiler.
  if(_CMAKE_USER_C_COMPILER_PATH)
    find_program(CMAKE_CXX_COMPILER NAMES ${CMAKE_CXX_COMPILER_LIST} PATHS ${_CMAKE_USER_C_COMPILER_PATH} DOC "C++ compiler" NO_DEFAULT_PATH)
  endif(_CMAKE_USER_C_COMPILER_PATH)
  find_program(CMAKE_CXX_COMPILER NAMES ${CMAKE_CXX_COMPILER_LIST} DOC "C++ compiler")

  if(CMAKE_CXX_COMPILER_INIT AND NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_INIT}" CACHE FILEPATH "C++ compiler" FORCE)
  endif(CMAKE_CXX_COMPILER_INIT AND NOT CMAKE_CXX_COMPILER)
else(NOT CMAKE_CXX_COMPILER)

# we only get here if CMAKE_CXX_COMPILER was specified using -D or a pre-made CMakeCache.txt
# (e.g. via ctest) or set in CMAKE_TOOLCHAIN_FILE
#
# if CMAKE_CXX_COMPILER is a list of length 2, use the first item as
# CMAKE_CXX_COMPILER and the 2nd one as CMAKE_CXX_COMPILER_ARG1

  list(LENGTH CMAKE_CXX_COMPILER _CMAKE_CXX_COMPILER_LIST_LENGTH)
  if("${_CMAKE_CXX_COMPILER_LIST_LENGTH}" EQUAL 2)
    list(GET CMAKE_CXX_COMPILER 1 CMAKE_CXX_COMPILER_ARG1)
    list(GET CMAKE_CXX_COMPILER 0 CMAKE_CXX_COMPILER)
  endif("${_CMAKE_CXX_COMPILER_LIST_LENGTH}" EQUAL 2)

# if a compiler was specified by the user but without path,
# now try to find it with the full path
# if it is found, force it into the cache,
# if not, don't overwrite the setting (which was given by the user) with "NOTFOUND"
# if the CXX compiler already had a path, reuse it for searching the C compiler
  get_filename_component(_CMAKE_USER_CXX_COMPILER_PATH "${CMAKE_CXX_COMPILER}" PATH)
  if(NOT _CMAKE_USER_CXX_COMPILER_PATH)
    find_program(CMAKE_CXX_COMPILER_WITH_PATH NAMES ${CMAKE_CXX_COMPILER})
    mark_as_advanced(CMAKE_CXX_COMPILER_WITH_PATH)
    if(CMAKE_CXX_COMPILER_WITH_PATH)
      set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER_WITH_PATH} CACHE STRING "CXX compiler" FORCE)
    endif(CMAKE_CXX_COMPILER_WITH_PATH)
  endif(NOT _CMAKE_USER_CXX_COMPILER_PATH)
endif(NOT CMAKE_CXX_COMPILER)
mark_as_advanced(CMAKE_CXX_COMPILER)

if(NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_CXX_COMPILER}" PATH)
endif(NOT _CMAKE_TOOLCHAIN_LOCATION)

# This block was used before the compiler was identified by building a
# source file.  Unless g++ crashes when building a small C++
# executable this should no longer be needed.
#
# The g++ that comes with BeOS 5 segfaults if you run "g++ -E"
#  ("gcc -E" is fine), which throws up a system dialog box that hangs cmake
#  until the user clicks "OK"...so for now, we just assume it's g++.
# if(BEOS)
#   set(CMAKE_COMPILER_IS_GNUCXX 1)
#   set(CMAKE_COMPILER_IS_GNUCXX_RUN 1)
# endif(BEOS)

# Build a small source file to identify the compiler.
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  set(CMAKE_CXX_COMPILER_ID_RUN 1)
  set(CMAKE_CXX_PLATFORM_ID "Windows")
  set(CMAKE_CXX_COMPILER_ID "MSVC")
endif(${CMAKE_GENERATOR} MATCHES "Visual Studio")
if(NOT CMAKE_CXX_COMPILER_ID_RUN)
  set(CMAKE_CXX_COMPILER_ID_RUN 1)

  # Each entry in this list is a set of extra flags to try
  # adding to the compile line to see if it helps produce
  # a valid identification file.
  set(CMAKE_CXX_COMPILER_ID_TEST_FLAGS
    # Try compiling to an object file only.
    "-c"
    )

  # Try to identify the compiler.
  set(CMAKE_CXX_COMPILER_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_CXX_COMPILER_ID_PLATFORM_CONTENT)
  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(CXX CXXFLAGS CMakeCXXCompilerId.cpp)

  # Set old compiler and platform id variables.
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCXX 1)
  endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_CXX_PLATFORM_ID}" MATCHES "MinGW")
    set(CMAKE_COMPILER_IS_MINGW 1)
  elseif("${CMAKE_CXX_PLATFORM_ID}" MATCHES "Cygwin")
    set(CMAKE_COMPILER_IS_CYGWIN 1)
  endif("${CMAKE_CXX_PLATFORM_ID}" MATCHES "MinGW")
endif(NOT CMAKE_CXX_COMPILER_ID_RUN)

# if we have a g++ cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-g++, arm-elf-g++ or i586-mingw32msvc-g++ , optionally
# with a 3-component version number at the end (e.g. arm-eabi-gcc-4.5.2).
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names lile
# "arm-unknown-nto-qnx6.3.0-gcc.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"
if(CMAKE_CROSSCOMPILING
    AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU"
    AND NOT _CMAKE_TOOLCHAIN_PREFIX)
  get_filename_component(COMPILER_BASENAME "${CMAKE_CXX_COMPILER}" NAME)
  if(COMPILER_BASENAME MATCHES "^(.+-)[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif(COMPILER_BASENAME MATCHES "^(.+-)[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")

  # if "llvm-" is part of the prefix, remove it, since llvm doesn't have its own binutils
  # but uses the regular ar, objcopy, etc. (instead of llvm-objcopy etc.)
  if("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")

endif(CMAKE_CROSSCOMPILING
    AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU"
    AND NOT _CMAKE_TOOLCHAIN_PREFIX)

include(CMakeFindBinUtils)
if(MSVC_CXX_ARCHITECTURE_ID)
  set(SET_MSVC_CXX_ARCHITECTURE_ID
    "set(MSVC_CXX_ARCHITECTURE_ID ${MSVC_CXX_ARCHITECTURE_ID})")
endif(MSVC_CXX_ARCHITECTURE_ID)
# configure all variables set in this file
configure_file(${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake
  @ONLY IMMEDIATE # IMMEDIATE must be here for compatibility mode <= 2.0
  )

set(CMAKE_CXX_COMPILER_ENV_VAR "CXX")
