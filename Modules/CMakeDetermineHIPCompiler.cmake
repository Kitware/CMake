# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)
include(${CMAKE_ROOT}/Modules/CMakeParseImplicitLinkInfo.cmake)

if( NOT ( ("${CMAKE_GENERATOR}" MATCHES "Make") OR
          ("${CMAKE_GENERATOR}" MATCHES "Ninja") ) )
  message(FATAL_ERROR "HIP language not currently supported by \"${CMAKE_GENERATOR}\" generator")
endif()


if(NOT CMAKE_HIP_COMPILER)
  set(CMAKE_HIP_COMPILER_INIT NOTFOUND)

  # prefer the environment variable HIPCXX
  if(NOT $ENV{HIPCXX} STREQUAL "")
    get_filename_component(CMAKE_HIP_COMPILER_INIT $ENV{HIPCXX} PROGRAM PROGRAM_ARGS CMAKE_HIP_FLAGS_ENV_INIT)
    if(CMAKE_HIP_FLAGS_ENV_INIT)
      set(CMAKE_HIP_COMPILER_ARG1 "${CMAKE_HIP_FLAGS_ENV_INIT}" CACHE STRING "Arguments to CXX compiler")
    endif()
    if(NOT EXISTS ${CMAKE_HIP_COMPILER_INIT})
      message(FATAL_ERROR "Could not find compiler set in environment variable HIPCXX:\n$ENV{HIPCXX}.\n${CMAKE_HIP_COMPILER_INIT}")
    endif()
  endif()

  # finally list compilers to try
  if(NOT CMAKE_HIP_COMPILER_INIT)
    set(CMAKE_HIP_COMPILER_LIST hipcc clang++)
  endif()

  _cmake_find_compiler(HIP)
else()
  _cmake_find_compiler_path(HIP)
endif()

mark_as_advanced(CMAKE_HIP_COMPILER)

# Build a small source file to identify the compiler.
if(NOT CMAKE_HIP_COMPILER_ID_RUN)
  set(CMAKE_HIP_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  set(CMAKE_HIP_COMPILER_ID)
  set(CMAKE_HIP_PLATFORM_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_HIP_COMPILER_ID_PLATFORM_CONTENT)

  list(APPEND CMAKE_HIP_COMPILER_ID_TEST_FLAGS_FIRST "-v")

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(HIP HIPFLAGS CMakeHIPCompilerId.hip)

  _cmake_find_compiler_sysroot(HIP)

endif()

if (NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_HIP_COMPILER}" PATH)
endif ()

set(_CMAKE_PROCESSING_LANGUAGE "HIP")
include(CMakeFindBinUtils)
include(Compiler/${CMAKE_HIP_COMPILER_ID}-FindBinUtils OPTIONAL)
unset(_CMAKE_PROCESSING_LANGUAGE)

if(CMAKE_HIP_COMPILER_SYSROOT)
  string(CONCAT _SET_CMAKE_HIP_COMPILER_SYSROOT
    "set(CMAKE_HIP_COMPILER_SYSROOT \"${CMAKE_HIP_COMPILER_SYSROOT}\")\n"
    "set(CMAKE_COMPILER_SYSROOT \"${CMAKE_HIP_COMPILER_SYSROOT}\")")
else()
  set(_SET_CMAKE_HIP_COMPILER_SYSROOT "")
endif()

if(CMAKE_HIP_COMPILER_ARCHITECTURE_ID)
  set(_SET_CMAKE_HIP_COMPILER_ARCHITECTURE_ID
    "set(CMAKE_HIP_COMPILER_ARCHITECTURE_ID ${CMAKE_HIP_COMPILER_ARCHITECTURE_ID})")
else()
  set(_SET_CMAKE_HIP_COMPILER_ARCHITECTURE_ID "")
endif()

if(MSVC_HIP_ARCHITECTURE_ID)
  set(SET_MSVC_HIP_ARCHITECTURE_ID
    "set(MSVC_HIP_ARCHITECTURE_ID ${MSVC_HIP_ARCHITECTURE_ID})")
endif()

if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
  # Analyze output from hipcc to get the current GPU architecture.
  if(CMAKE_HIP_COMPILER_PRODUCED_OUTPUT MATCHES " -target-cpu ([a-z0-9]+) ")
    set(CMAKE_HIP_ARCHITECTURES "${CMAKE_MATCH_1}" CACHE STRING "HIP architectures")
  else()
    message(FATAL_ERROR "Failed to find a working HIP architecture.")
  endif()
endif()

# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeHIPCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeHIPCompiler.cmake
  @ONLY
  )
set(CMAKE_HIP_COMPILER_ENV_VAR "HIPCXX")
