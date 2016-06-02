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

# This module is shared by multiple languages; use include blocker.
if(__ANDROID_COMPILER_COMMON)
  return()
endif()
set(__ANDROID_COMPILER_COMMON 1)

# The NDK toolchain configuration files at:
#
#   <ndk>/[build/core/]toolchains/*/setup.mk
#
# contain logic to set TARGET_CFLAGS and TARGET_LDFLAGS (and debug/release
# variants) to tell their build system what flags to pass for each ABI.
# We need to produce the same flags here to produce compatible binaries.
# We initialize these variables here and set them in the compiler-specific
# modules that include this one.  Then we use them in the macro below when
# it is called.
set(_ANDROID_ABI_INIT_CFLAGS "")
set(_ANDROID_ABI_INIT_CFLAGS_DEBUG "")
set(_ANDROID_ABI_INIT_CFLAGS_RELEASE "")
set(_ANDROID_ABI_INIT_LDFLAGS "")

macro(__android_compiler_common lang)
  if(_ANDROID_ABI_INIT_CFLAGS)
    string(APPEND CMAKE_${lang}_FLAGS_INIT " ${_ANDROID_ABI_INIT_CFLAGS}")
  endif()
  if(_ANDROID_ABI_INIT_CFLAGS_DEBUG)
    string(APPEND CMAKE_${lang}_FLAGS_DEBUG_INIT " ${_ANDROID_ABI_INIT_CFLAGS_DEBUG}")
  endif()
  if(_ANDROID_ABI_INIT_CFLAGS_RELEASE)
    string(APPEND CMAKE_${lang}_FLAGS_RELEASE_INIT " ${_ANDROID_ABI_INIT_CFLAGS_RELEASE}")
    string(APPEND CMAKE_${lang}_FLAGS_MINSIZEREL_INIT " ${_ANDROID_ABI_INIT_CFLAGS_RELEASE}")
    string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT " ${_ANDROID_ABI_INIT_CFLAGS_RELEASE}")
  endif()
  if(_ANDROID_ABI_INIT_LDFLAGS)
    foreach(t EXE SHARED MODULE)
      string(APPEND CMAKE_${t}_LINKER_FLAGS_INIT " ${_ANDROID_ABI_INIT_LDFLAGS}")
    endforeach()
  endif()
endmacro()
