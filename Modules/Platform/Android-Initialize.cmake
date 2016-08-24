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

# When CMAKE_SYSTEM_NAME is "Android", CMakeSystemSpecificInitialize loads this
# module.

# Support for NVIDIA Nsight Tegra Visual Studio Edition was previously
# implemented in the CMake VS IDE generators.  Avoid interfering with
# that functionality for now.  Later we may try to integrate this.
if(CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android")
  return()
endif()

# Commonly used Android toolchain files that pre-date CMake upstream support
# set CMAKE_SYSTEM_VERSION to 1.  Avoid interfering with them.
if(CMAKE_SYSTEM_VERSION EQUAL 1)
  return()
endif()

if(NOT CMAKE_SYSROOT)
  if(CMAKE_ANDROID_NDK)
    set(CMAKE_SYSROOT "${CMAKE_ANDROID_NDK}/platforms/android-${CMAKE_SYSTEM_VERSION}/arch-${CMAKE_ANDROID_ARCH}")
  elseif(CMAKE_ANDROID_STANDALONE_TOOLCHAIN)
    set(CMAKE_SYSROOT "${CMAKE_ANDROID_STANDALONE_TOOLCHAIN}/sysroot")
  endif()
endif()

if(CMAKE_SYSROOT)
  if(NOT IS_DIRECTORY "${CMAKE_SYSROOT}")
    message(FATAL_ERROR
      "Android: The system root directory needed for the selected Android version and architecture does not exist:\n"
      "  ${CMAKE_SYSROOT}\n"
      )
  endif()
else()
  message(FATAL_ERROR
    "Android: No CMAKE_SYSROOT was selected."
    )
endif()

set(CMAKE_BUILD_TYPE_INIT Debug)
