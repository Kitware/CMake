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
    if("$ENV{HIPCXX}" MATCHES "hipcc")
      message(FATAL_ERROR
        "The HIPCXX environment variable is set to the hipcc wrapper:\n"
        " $ENV{HIPCXX}\n"
        "This is not supported.  Use Clang directly, or let CMake pick a default."
        )
    endif()
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
    set(CMAKE_HIP_COMPILER_LIST clang++)

    # Look for the Clang coming with ROCm to support HIP.
    execute_process(COMMAND hipconfig --hipclangpath
      OUTPUT_VARIABLE _CMAKE_HIPCONFIG_CLANGPATH
      RESULT_VARIABLE _CMAKE_HIPCONFIG_RESULT
    )
    if(_CMAKE_HIPCONFIG_RESULT EQUAL 0 AND EXISTS "${_CMAKE_HIPCONFIG_CLANGPATH}")
      set(CMAKE_HIP_COMPILER_HINTS "${_CMAKE_HIPCONFIG_CLANGPATH}")
    endif()
  endif()

  _cmake_find_compiler(HIP)
elseif(CMAKE_HIP_COMPILER MATCHES "hipcc")
  message(FATAL_ERROR
    "CMAKE_HIP_COMPILER is set to the hipcc wrapper:\n"
    " ${CMAKE_HIP_COMPILER}\n"
    "This is not supported.  Use Clang directly, or let CMake pick a default."
    )
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

if(NOT CMAKE_HIP_COMPILER_ROCM_ROOT AND CMAKE_HIP_COMPILER_ID STREQUAL "Clang")
   execute_process(COMMAND "${CMAKE_HIP_COMPILER}" -v -print-targets
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _CMAKE_HIP_COMPILER_RESULT
    OUTPUT_VARIABLE _CMAKE_HIP_COMPILER_STDOUT
    ERROR_VARIABLE _CMAKE_HIP_COMPILER_STDERR
    )

  if(_CMAKE_HIP_COMPILER_RESULT EQUAL 0 AND _CMAKE_HIP_COMPILER_STDERR MATCHES "Found HIP installation: *([^,]*)[,\n]")
    set(CMAKE_HIP_COMPILER_ROCM_ROOT "${CMAKE_MATCH_1}")
    file(TO_CMAKE_PATH "${CMAKE_HIP_COMPILER_ROCM_ROOT}" CMAKE_HIP_COMPILER_ROCM_ROOT)
  endif()
endif()
if(NOT CMAKE_HIP_COMPILER_ROCM_ROOT)
  execute_process(
    COMMAND hipconfig --rocmpath
    OUTPUT_VARIABLE _CMAKE_HIPCONFIG_ROCMPATH
    RESULT_VARIABLE _CMAKE_HIPCONFIG_RESULT
    )
  if(_CMAKE_HIPCONFIG_RESULT EQUAL 0 AND EXISTS "${_CMAKE_HIPCONFIG_ROCMPATH}")
    set(CMAKE_HIP_COMPILER_ROCM_ROOT "${_CMAKE_HIPCONFIG_ROCMPATH}")
  endif()
endif()
if(NOT CMAKE_HIP_COMPILER_ROCM_ROOT)
  message(FATAL_ERROR "Failed to find ROCm root directory.")
endif()
if(NOT EXISTS "${CMAKE_HIP_COMPILER_ROCM_ROOT}/lib/cmake/hip-lang/hip-lang-config.cmake")
  message(FATAL_ERROR
    "The ROCm root directory:\n"
    " ${CMAKE_HIP_COMPILER_ROCM_ROOT}\n"
    "does not contain the HIP runtime CMake package, expected at:\n"
    " ${CMAKE_HIP_COMPILER_ROCM_ROOT}/lib/cmake/hip-lang/hip-lang-config.cmake\n"
    )
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
  # Use 'rocm_agent_enumerator' to get the current GPU architecture.
  set(_CMAKE_HIP_ARCHITECTURES)
  find_program(_CMAKE_HIP_ROCM_AGENT_ENUMERATOR
    NAMES rocm_agent_enumerator
    HINTS "${CMAKE_HIP_COMPILER_ROCM_ROOT}/bin"
    NO_CACHE)
  if(_CMAKE_HIP_ROCM_AGENT_ENUMERATOR)
    execute_process(COMMAND "${_CMAKE_HIP_ROCM_AGENT_ENUMERATOR}" -t GPU
      RESULT_VARIABLE _CMAKE_ROCM_AGENT_ENUMERATOR_RESULT
      OUTPUT_VARIABLE _CMAKE_ROCM_AGENT_ENUMERATOR_STDOUT
      ERROR_VARIABLE  _CMAKE_ROCM_AGENT_ENUMERATOR_STDERR
    )
    if(_CMAKE_ROCM_AGENT_ENUMERATOR_RESULT EQUAL 0)
      separate_arguments(_hip_archs NATIVE_COMMAND "${_CMAKE_ROCM_AGENT_ENUMERATOR_STDOUT}")
      foreach(_hip_arch ${_hip_archs})
        if(_hip_arch STREQUAL "gfx000")
          continue()
        endif()
        string(FIND ${_hip_arch} ":" pos)
        if(NOT pos STREQUAL "-1")
          string(SUBSTRING ${_hip_arch} 0 ${pos} _hip_arch)
        endif()
        list(APPEND _CMAKE_HIP_ARCHITECTURES "${_hip_arch}")
      endforeach()
    endif()
    unset(_CMAKE_ROCM_AGENT_ENUMERATOR_RESULT)
    unset(_CMAKE_ROCM_AGENT_ENUMERATOR_STDOUT)
    unset(_CMAKE_ROCM_AGENT_ENUMERATOR_STDERR)
  endif()
  unset(_CMAKE_HIP_ROCM_AGENT_ENUMERATOR)
  if(_CMAKE_HIP_ARCHITECTURES)
    set(CMAKE_HIP_ARCHITECTURES "${_CMAKE_HIP_ARCHITECTURES}" CACHE STRING "HIP architectures")
  elseif(CMAKE_HIP_COMPILER_PRODUCED_OUTPUT MATCHES " -target-cpu ([a-z0-9]+) ")
    set(CMAKE_HIP_ARCHITECTURES "${CMAKE_MATCH_1}" CACHE STRING "HIP architectures")
  else()
    message(FATAL_ERROR "Failed to find a default HIP architecture.")
  endif()
  unset(_CMAKE_HIP_ARCHITECTURES)
endif()

# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeHIPCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeHIPCompiler.cmake
  @ONLY
  )
set(CMAKE_HIP_COMPILER_ENV_VAR "HIPCXX")
