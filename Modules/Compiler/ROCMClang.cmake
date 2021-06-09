# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
include_guard()

include(Compiler/CMakeCommonCompilerMacros)

macro(__compiler_rocmclang lang)

  set(CMAKE_${lang}_VERBOSE_FLAG "-v")

  if(NOT "x${CMAKE_${lang}_SIMULATE_ID}" STREQUAL "xMSVC")
    # Feature flags.
    set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "-fPIC")
    set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "-fPIE")
    set(CMAKE_HIP_COMPILE_OPTIONS_VISIBILITY -fvisibility=)

    string(APPEND CMAKE_HIP_FLAGS_INIT " ")
    string(APPEND CMAKE_HIP_FLAGS_DEBUG_INIT " -g")
    string(APPEND CMAKE_HIP_FLAGS_RELEASE_INIT " -O3 -DNDEBUG")
    string(APPEND CMAKE_HIP_FLAGS_MINSIZEREL_INIT " -Os -DNDEBUG")
    string(APPEND CMAKE_HIP_FLAGS_RELWITHDEBINFO_INIT " -O2 -g -DNDEBUG")
  endif()

  set(CMAKE_SHARED_LIBRARY_CREATE_HIP_FLAGS -shared)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_HIP "-isystem ")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_INCLUDES 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_RESPONSE_FILE_FLAG "@")
  set(CMAKE_${lang}_RESPONSE_FILE_LINK_FLAG "@")
endmacro()
