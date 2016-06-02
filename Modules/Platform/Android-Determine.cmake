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

# Save the Android-specific information in CMakeSystem.cmake.
set(CMAKE_SYSTEM_CUSTOM_CODE "
set(CMAKE_ANDROID_NDK \"${CMAKE_ANDROID_NDK}\")
")
