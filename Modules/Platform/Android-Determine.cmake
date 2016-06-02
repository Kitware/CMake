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

# https://developer.android.com/ndk/guides/abis.html

set(_ANDROID_ABI_arm64-v8a_PROC   "aarch64")
set(_ANDROID_ABI_arm64-v8a_ARCH   "arm64")
set(_ANDROID_ABI_armeabi-v7a_PROC "armv7-a")
set(_ANDROID_ABI_armeabi-v7a_ARCH "arm")
set(_ANDROID_ABI_armeabi-v6_PROC  "armv6")
set(_ANDROID_ABI_armeabi-v6_ARCH  "arm")
set(_ANDROID_ABI_armeabi_PROC     "armv5te")
set(_ANDROID_ABI_armeabi_ARCH     "arm")
set(_ANDROID_ABI_mips_PROC        "mips")
set(_ANDROID_ABI_mips_ARCH        "mips")
set(_ANDROID_ABI_mips64_PROC      "mips64")
set(_ANDROID_ABI_mips64_ARCH      "mips64")
set(_ANDROID_ABI_x86_PROC         "i686")
set(_ANDROID_ABI_x86_ARCH         "x86")
set(_ANDROID_ABI_x86_64_PROC      "x86_64")
set(_ANDROID_ABI_x86_64_ARCH      "x86_64")

set(_ANDROID_PROC_aarch64_ARCH_ABI "arm64-v8a")
set(_ANDROID_PROC_armv7-a_ARCH_ABI "armeabi-v7a")
set(_ANDROID_PROC_armv6_ARCH_ABI   "armeabi-v6")
set(_ANDROID_PROC_armv5te_ARCH_ABI "armeabi")
set(_ANDROID_PROC_i686_ARCH_ABI    "x86")
set(_ANDROID_PROC_mips_ARCH_ABI    "mips")
set(_ANDROID_PROC_mips64_ARCH_ABI  "mips64")
set(_ANDROID_PROC_x86_64_ARCH_ABI  "x86_64")

# Validate inputs.
if(CMAKE_ANDROID_ARCH_ABI AND NOT DEFINED "_ANDROID_ABI_${CMAKE_ANDROID_ARCH_ABI}_PROC")
  message(FATAL_ERROR "Android: Unknown ABI CMAKE_ANDROID_ARCH_ABI='${CMAKE_ANDROID_ARCH_ABI}'.")
endif()
if(CMAKE_SYSTEM_PROCESSOR AND NOT DEFINED "_ANDROID_PROC_${CMAKE_SYSTEM_PROCESSOR}_ARCH_ABI")
  message(FATAL_ERROR "Android: Unknown processor CMAKE_SYSTEM_PROCESSOR='${CMAKE_SYSTEM_PROCESSOR}'.")
endif()

# Select an ABI.
if(NOT CMAKE_ANDROID_ARCH_ABI)
  if(CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_ANDROID_ARCH_ABI "${_ANDROID_PROC_${CMAKE_SYSTEM_PROCESSOR}_ARCH_ABI}")
  else()
    # https://developer.android.com/ndk/guides/application_mk.html
    # Default is the oldest ARM ABI.
    set(CMAKE_ANDROID_ARCH_ABI "armeabi")
  endif()
endif()
set(CMAKE_ANDROID_ARCH "${_ANDROID_ABI_${CMAKE_ANDROID_ARCH_ABI}_ARCH}")

# Select a processor.
if(NOT CMAKE_SYSTEM_PROCESSOR)
  set(CMAKE_SYSTEM_PROCESSOR "${_ANDROID_ABI_${CMAKE_ANDROID_ARCH_ABI}_PROC}")
endif()

# If the user specified both an ABI and a processor then they might not match.
if(NOT _ANDROID_ABI_${CMAKE_ANDROID_ARCH_ABI}_PROC STREQUAL CMAKE_SYSTEM_PROCESSOR)
  message(FATAL_ERROR "Android: The specified CMAKE_ANDROID_ARCH_ABI='${CMAKE_ANDROID_ARCH_ABI}' and CMAKE_SYSTEM_PROCESSOR='${CMAKE_SYSTEM_PROCESSOR}' is not a valid combination.")
endif()

# Save the Android-specific information in CMakeSystem.cmake.
set(CMAKE_SYSTEM_CUSTOM_CODE "
set(CMAKE_ANDROID_NDK \"${CMAKE_ANDROID_NDK}\")
set(CMAKE_ANDROID_ARCH \"${CMAKE_ANDROID_ARCH}\")
set(CMAKE_ANDROID_ARCH_ABI \"${CMAKE_ANDROID_ARCH_ABI}\")
")

# Select an ARM variant.
if(CMAKE_ANDROID_ARCH_ABI MATCHES "^armeabi")
  if(CMAKE_ANDROID_ARM_MODE)
    set(CMAKE_ANDROID_ARM_MODE 1)
  else()
    set(CMAKE_ANDROID_ARM_MODE 0)
  endif()
  string(APPEND CMAKE_SYSTEM_CUSTOM_CODE
    "set(CMAKE_ANDROID_ARM_MODE \"${CMAKE_ANDROID_ARM_MODE}\")\n"
    )
elseif(DEFINED CMAKE_ANDROID_ARM_MODE)
  message(FATAL_ERROR "Android: CMAKE_ANDROID_ARM_MODE is set but is valid only for 'armeabi' architectures.")
endif()

if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
  if(CMAKE_ANDROID_ARM_NEON)
    set(CMAKE_ANDROID_ARM_NEON 1)
  else()
    set(CMAKE_ANDROID_ARM_NEON 0)
  endif()
  string(APPEND CMAKE_SYSTEM_CUSTOM_CODE
    "set(CMAKE_ANDROID_ARM_NEON \"${CMAKE_ANDROID_ARM_NEON}\")\n"
    )
elseif(DEFINED CMAKE_ANDROID_ARM_NEON)
  message(FATAL_ERROR "Android: CMAKE_ANDROID_ARM_NEON is set but is valid only for 'armeabi-v7a' architecture.")
endif()

# Report the chosen architecture.
message(STATUS "Android: Targeting API '${CMAKE_SYSTEM_VERSION}' with architecture '${CMAKE_ANDROID_ARCH}', ABI '${CMAKE_ANDROID_ARCH_ABI}', and processor '${CMAKE_SYSTEM_PROCESSOR}'")
