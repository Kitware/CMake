
#=============================================================================
# Copyright 2002-2016 Kitware, Inc.
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
if(__COMPILER_INTEL)
  return()
endif()
set(__COMPILER_INTEL 1)

if(CMAKE_HOST_WIN32)
  # MSVC-like
  macro(__compiler_intel lang)
  endmacro()
else()
  # GNU-like
  macro(__compiler_intel lang)
    set(CMAKE_${lang}_VERBOSE_FLAG "-v")

    set(CMAKE_${lang}_FLAGS_INIT "")
    set(CMAKE_${lang}_FLAGS_DEBUG_INIT "-g")
    set(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "-Os")
    set(CMAKE_${lang}_FLAGS_RELEASE_INIT "-O3")
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
  endmacro()
endif()
