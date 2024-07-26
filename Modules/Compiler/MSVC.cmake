# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

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
endmacro()
