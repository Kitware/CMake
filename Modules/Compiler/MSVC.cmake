# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# This module is shared by multiple languages; use include blocker.
if(__COMPILER_MSVC)
  return()
endif()
set(__COMPILER_MSVC 1)

macro(__compiler_msvc lang)
  set(CMAKE_${lang}_CLANG_TIDY_DRIVER_MODE "cl")
  set(CMAKE_${lang}_INCLUDE_WHAT_YOU_USE_DRIVER_MODE "cl")
  set(CMAKE_${lang}_COMPILE_OPTIONS_WARNING_AS_ERROR "-WX")

  # /JMC "Just My Code" is only supported by MSVC 19.05 onward.
  if (CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 19.05)
    set(CMAKE_${lang}_COMPILE_OPTIONS_JMC "-JMC")
  endif()

  # The `/external:I` flag was made non-experimental in 19.29.30036.3.
  if (CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30036.3)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-external:I")
    set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang}_WARNING "-external:W0 ")
  endif ()

  # '/PD' dumps the predefined macros, but it needs the conforming
  # preprocessor and one of '/E', '/EP' or '/P'.  VS 2019 16.7 and older
  # ignore it with warning D9002, so require 16.8.  '/EP' also echoes the
  # preprocessed source, hence the empty translation unit.
  if("${lang}" STREQUAL "CXX" AND
      CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28)
    set(CMAKE_${lang}_COMPILER_PREDEFINES_COMMAND "${CMAKE_${lang}_COMPILER}")
    if(CMAKE_${lang}_COMPILER_ARG1)
      separate_arguments(_COMPILER_ARGS NATIVE_COMMAND "${CMAKE_${lang}_COMPILER_ARG1}")
      list(APPEND CMAKE_${lang}_COMPILER_PREDEFINES_COMMAND ${_COMPILER_ARGS})
      unset(_COMPILER_ARGS)
    endif()
    list(APPEND CMAKE_${lang}_COMPILER_PREDEFINES_COMMAND
      "-nologo" "-w" "-Zc:preprocessor" "-PD" "-EP"
      "${CMAKE_ROOT}/Modules/CMakeCXXCompilerPredefines.cpp")
  endif()

  set(CMAKE_${lang}_LINK_MODE LINKER)
endmacro()
