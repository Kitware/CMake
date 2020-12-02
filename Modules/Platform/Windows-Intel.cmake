# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_INTEL)
  return()
endif()
set(__WINDOWS_INTEL 1)


if (CMAKE_GENERATOR MATCHES "^Ninja")
  # retrieve ninja version to enable dependencies configuration
  # against Ninja capabilities
  execute_process(COMMAND "${CMAKE_MAKE_PROGRAM}" --version
    RESULT_VARIABLE _CMAKE_NINJA_RESULT
    OUTPUT_VARIABLE _CMAKE_NINJA_VERSION
    ERROR_VARIABLE _CMAKE_NINJA_VERSION)
  if (NOT _CMAKE_NINJA_RESULT AND _CMAKE_NINJA_VERSION MATCHES "[0-9]+(\\.[0-9]+)*")
    set (_CMAKE_NINJA_VERSION "${CMAKE_MATCH_0}")
  endif()
  unset(_CMAKE_NINJA_RESULT)
endif()

include(Platform/Windows-MSVC)
macro(__windows_compiler_intel lang)
  __windows_compiler_msvc(${lang})
  string(REPLACE "<CMAKE_AR>" "xilib" CMAKE_${lang}_CREATE_STATIC_LIBRARY "${CMAKE_${lang}_CREATE_STATIC_LIBRARY}")
  foreach(rule CREATE_SHARED_LIBRARY CREATE_SHARED_MODULE LINK_EXECUTABLE)
    string(REPLACE "<CMAKE_LINKER>" "xilink" CMAKE_${lang}_${rule} "${CMAKE_${lang}_${rule}}")
  endforeach()
endmacro()
