#=============================================================================
# Copyright 2015-2016 Kitware, Inc.
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

# When CMAKE_SYSTEM_NAME is "Android", CMakeDetermineSystem loads this module.
# This module detects platform-wide information about the Android target
# in order to store it in "CMakeSystem.cmake".

# Support for NVIDIA Nsight Tegra Visual Studio Edition was previously
# implemented in the CMake VS IDE generators.  Avoid interfering with
# that functionality for now.  Later we may try to integrate this.
if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
  return()
endif()

# Find the Android NDK.
if(CMAKE_ANDROID_NDK)
  if(NOT IS_DIRECTORY "${CMAKE_ANDROID_NDK}")
    message(FATAL_ERROR
      "Android: The NDK root directory specified by CMAKE_ANDROID_NDK:\n"
      "  ${CMAKE_ANDROID_NDK}\n"
      "does not exist."
      )
  endif()
else()
  if(IS_DIRECTORY "$ENV{ANDROID_NDK_ROOT}")
    file(TO_CMAKE_PATH "$ENV{ANDROID_NDK_ROOT}" CMAKE_ANDROID_NDK)
  endif()
  # TODO: Search harder for the NDK.
endif()

if(NOT CMAKE_ANDROID_NDK)
  message(FATAL_ERROR "Android: The NDK root directory was not found.")
endif()

# Select an API.
if(CMAKE_SYSTEM_VERSION)
  set(_ANDROID_API_VAR CMAKE_SYSTEM_VERSION)
elseif(CMAKE_ANDROID_API)
  set(CMAKE_SYSTEM_VERSION "${CMAKE_ANDROID_API}")
  set(_ANDROID_API_VAR CMAKE_ANDROID_API)
endif()
if(CMAKE_SYSTEM_VERSION)
  if(CMAKE_ANDROID_API AND NOT "x${CMAKE_ANDROID_API}" STREQUAL "x${CMAKE_SYSTEM_VERSION}")
    message(FATAL_ERROR
      "Android: The API specified by CMAKE_ANDROID_API='${CMAKE_ANDROID_API}' is not consistent with CMAKE_SYSTEM_VERSION='${CMAKE_SYSTEM_VERSION}'."
      )
  endif()
  if(CMAKE_ANDROID_NDK AND NOT IS_DIRECTORY "${CMAKE_ANDROID_NDK}/platforms/android-${CMAKE_SYSTEM_VERSION}")
    message(FATAL_ERROR
      "Android: The API specified by ${_ANDROID_API_VAR}='${${_ANDROID_API_VAR}}' does not exist in the NDK.  "
      "The directory:\n"
      "  ${CMAKE_ANDROID_NDK}/platforms/android-${CMAKE_SYSTEM_VERSION}\n"
      "does not exist."
      )
  endif()
elseif(CMAKE_ANDROID_NDK)
  file(GLOB _ANDROID_APIS_1 RELATIVE "${CMAKE_ANDROID_NDK}/platforms" "${CMAKE_ANDROID_NDK}/platforms/android-[0-9]")
  file(GLOB _ANDROID_APIS_2 RELATIVE "${CMAKE_ANDROID_NDK}/platforms" "${CMAKE_ANDROID_NDK}/platforms/android-[0-9][0-9]")
  list(SORT _ANDROID_APIS_1)
  list(SORT _ANDROID_APIS_2)
  set(_ANDROID_APIS ${_ANDROID_APIS_1} ${_ANDROID_APIS_2})
  unset(_ANDROID_APIS_1)
  unset(_ANDROID_APIS_2)
  if(_ANDROID_APIS STREQUAL "")
    message(FATAL_ERROR
      "Android: No APIs found in the NDK.  No\n"
      "  ${CMAKE_ANDROID_NDK}/platforms/android-*\n"
      "directories exist."
      )
  endif()
  string(REPLACE "android-" "" _ANDROID_APIS "${_ANDROID_APIS}")
  list(REVERSE _ANDROID_APIS)
  list(GET _ANDROID_APIS 0 CMAKE_SYSTEM_VERSION)
  unset(_ANDROID_APIS)
endif()
if(NOT CMAKE_SYSTEM_VERSION MATCHES "^[0-9]+$")
  message(FATAL_ERROR "Android: The API specified by CMAKE_SYSTEM_VERSION='${CMAKE_SYSTEM_VERSION}' is not an integer.")
endif()

# Save the Android-specific information in CMakeSystem.cmake.
set(CMAKE_SYSTEM_CUSTOM_CODE "
set(CMAKE_ANDROID_NDK \"${CMAKE_ANDROID_NDK}\")
")

# Report the chosen architecture.
message(STATUS "Android: Targeting API '${CMAKE_SYSTEM_VERSION}'")
