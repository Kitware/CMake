
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

# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.
# use environment variable CC first if defined by user, next use
# the cmake variable CMAKE_GENERATOR_CC which can be defined by a generator
# as a default compiler
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-gcc, arm-elf-ar etc.). This works
# currently with the GNU crosscompilers.
#
# Sets the following variables:
#   CMAKE_C_COMPILER
#   CMAKE_AR
#   CMAKE_RANLIB
#   CMAKE_COMPILER_IS_GNUCC
#
# If not already set before, it also sets
#   _CMAKE_TOOLCHAIN_PREFIX

if(NOT CMAKE_C_COMPILER)
  set(CMAKE_CXX_COMPILER_INIT NOTFOUND)

  # prefer the environment variable CC
  if($ENV{CC} MATCHES ".+")
    get_filename_component(CMAKE_C_COMPILER_INIT $ENV{CC} PROGRAM PROGRAM_ARGS CMAKE_C_FLAGS_ENV_INIT)
    if(CMAKE_C_FLAGS_ENV_INIT)
      set(CMAKE_C_COMPILER_ARG1 "${CMAKE_C_FLAGS_ENV_INIT}" CACHE STRING "First argument to C compiler")
    endif(CMAKE_C_FLAGS_ENV_INIT)
    if(NOT EXISTS ${CMAKE_C_COMPILER_INIT})
      message(FATAL_ERROR "Could not find compiler set in environment variable CC:\n$ENV{CC}.")
    endif(NOT EXISTS ${CMAKE_C_COMPILER_INIT})
  endif($ENV{CC} MATCHES ".+")

  # next try prefer the compiler specified by the generator
  if(CMAKE_GENERATOR_CC)
    if(NOT CMAKE_C_COMPILER_INIT)
      set(CMAKE_C_COMPILER_INIT ${CMAKE_GENERATOR_CC})
    endif(NOT CMAKE_C_COMPILER_INIT)
  endif(CMAKE_GENERATOR_CC)

  # finally list compilers to try
  if(CMAKE_C_COMPILER_INIT)
    set(CMAKE_C_COMPILER_LIST ${CMAKE_C_COMPILER_INIT})
  else(CMAKE_C_COMPILER_INIT)
    set(CMAKE_C_COMPILER_LIST ${_CMAKE_TOOLCHAIN_PREFIX}gcc ${_CMAKE_TOOLCHAIN_PREFIX}cc cl bcc xlc)
  endif(CMAKE_C_COMPILER_INIT)

  # Find the compiler.
  if(_CMAKE_USER_CXX_COMPILER_PATH)
    find_program(CMAKE_C_COMPILER NAMES ${CMAKE_C_COMPILER_LIST} PATHS ${_CMAKE_USER_CXX_COMPILER_PATH} DOC "C compiler" NO_DEFAULT_PATH)
  endif(_CMAKE_USER_CXX_COMPILER_PATH)
  find_program(CMAKE_C_COMPILER NAMES ${CMAKE_C_COMPILER_LIST} DOC "C compiler")

  if(CMAKE_C_COMPILER_INIT AND NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER "${CMAKE_C_COMPILER_INIT}" CACHE FILEPATH "C compiler" FORCE)
  endif(CMAKE_C_COMPILER_INIT AND NOT CMAKE_C_COMPILER)
else(NOT CMAKE_C_COMPILER)

  # we only get here if CMAKE_C_COMPILER was specified using -D or a pre-made CMakeCache.txt
  # (e.g. via ctest) or set in CMAKE_TOOLCHAIN_FILE
  # if CMAKE_C_COMPILER is a list of length 2, use the first item as
  # CMAKE_C_COMPILER and the 2nd one as CMAKE_C_COMPILER_ARG1

  list(LENGTH CMAKE_C_COMPILER _CMAKE_C_COMPILER_LIST_LENGTH)
  if("${_CMAKE_C_COMPILER_LIST_LENGTH}" EQUAL 2)
    list(GET CMAKE_C_COMPILER 1 CMAKE_C_COMPILER_ARG1)
    list(GET CMAKE_C_COMPILER 0 CMAKE_C_COMPILER)
  endif("${_CMAKE_C_COMPILER_LIST_LENGTH}" EQUAL 2)

  # if a compiler was specified by the user but without path,
  # now try to find it with the full path
  # if it is found, force it into the cache,
  # if not, don't overwrite the setting (which was given by the user) with "NOTFOUND"
  # if the C compiler already had a path, reuse it for searching the CXX compiler
  get_filename_component(_CMAKE_USER_C_COMPILER_PATH "${CMAKE_C_COMPILER}" PATH)
  if(NOT _CMAKE_USER_C_COMPILER_PATH)
    find_program(CMAKE_C_COMPILER_WITH_PATH NAMES ${CMAKE_C_COMPILER})
    mark_as_advanced(CMAKE_C_COMPILER_WITH_PATH)
    if(CMAKE_C_COMPILER_WITH_PATH)
      set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER_WITH_PATH} CACHE STRING "C compiler" FORCE)
    endif(CMAKE_C_COMPILER_WITH_PATH)
  endif(NOT _CMAKE_USER_C_COMPILER_PATH)
endif(NOT CMAKE_C_COMPILER)
mark_as_advanced(CMAKE_C_COMPILER)

if(NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_C_COMPILER}" PATH)
endif(NOT _CMAKE_TOOLCHAIN_LOCATION)

# Build a small source file to identify the compiler.
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  set(CMAKE_C_COMPILER_ID_RUN 1)
  set(CMAKE_C_PLATFORM_ID "Windows")
  set(CMAKE_C_COMPILER_ID "MSVC")
endif(${CMAKE_GENERATOR} MATCHES "Visual Studio")

if(NOT CMAKE_C_COMPILER_ID_RUN)
  set(CMAKE_C_COMPILER_ID_RUN 1)

  # Each entry in this list is a set of extra flags to try
  # adding to the compile line to see if it helps produce
  # a valid identification file.
  set(CMAKE_C_COMPILER_ID_TEST_FLAGS
    # Try compiling to an object file only.
    "-c"

    # Try enabling ANSI mode on HP.
    "-Aa"
    )

  # Try to identify the compiler.
  set(CMAKE_C_COMPILER_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_C_COMPILER_ID_PLATFORM_CONTENT)
  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(C CFLAGS CMakeCCompilerId.c)

  # Set old compiler and platform id variables.
  if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCC 1)
  endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
  if("${CMAKE_C_PLATFORM_ID}" MATCHES "MinGW")
    set(CMAKE_COMPILER_IS_MINGW 1)
  elseif("${CMAKE_C_PLATFORM_ID}" MATCHES "Cygwin")
    set(CMAKE_COMPILER_IS_CYGWIN 1)
  endif("${CMAKE_C_PLATFORM_ID}" MATCHES "MinGW")
endif(NOT CMAKE_C_COMPILER_ID_RUN)

# If we have a gcc cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-gcc, arm-elf-gcc or i586-mingw32msvc-gcc, optionally
# with a 3-component version number at the end (e.g. arm-eabi-gcc-4.5.2).
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names lile
# "arm-unknown-nto-qnx6.3.0-gcc.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"
if(CMAKE_CROSSCOMPILING
    AND "${CMAKE_C_COMPILER_ID}" MATCHES "GNU"
    AND NOT _CMAKE_TOOLCHAIN_PREFIX)
  get_filename_component(COMPILER_BASENAME "${CMAKE_C_COMPILER}" NAME)
  if(COMPILER_BASENAME MATCHES "^(.+-)g?cc(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif(COMPILER_BASENAME MATCHES "^(.+-)g?cc(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")

  # if "llvm-" is part of the prefix, remove it, since llvm doesn't have its own binutils
  # but uses the regular ar, objcopy, etc. (instead of llvm-objcopy etc.)
  if("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  endif("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")

endif(CMAKE_CROSSCOMPILING
    AND "${CMAKE_C_COMPILER_ID}" MATCHES "GNU"
    AND NOT _CMAKE_TOOLCHAIN_PREFIX)




include(CMakeFindBinUtils)
if(MSVC_C_ARCHITECTURE_ID)
  set(SET_MSVC_C_ARCHITECTURE_ID
    "set(MSVC_C_ARCHITECTURE_ID ${MSVC_C_ARCHITECTURE_ID})")
endif(MSVC_C_ARCHITECTURE_ID)
# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in
  "${CMAKE_PLATFORM_ROOT_BIN}/CMakeCCompiler.cmake"
  @ONLY IMMEDIATE # IMMEDIATE must be here for compatibility mode <= 2.0
  )
set(CMAKE_C_COMPILER_ENV_VAR "CC")
