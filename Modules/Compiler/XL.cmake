
#=============================================================================
# Copyright 2002-2011 Kitware, Inc.
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
if(__COMPILER_XL)
  return()
endif()
set(__COMPILER_XL 1)

macro(__compiler_xl lang)
  # Feature flags.
  set(CMAKE_${lang}_VERBOSE_FLAG "-V")

  set(CMAKE_${lang}_FLAGS_DEBUG_INIT "-g")
  set(CMAKE_${lang}_FLAGS_RELEASE_INIT "-O")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "-O")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "-g")
  set(CMAKE_${lang}_CREATE_PREPROCESSED_SOURCE "<CMAKE_${lang}_COMPILER> <DEFINES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
  set(CMAKE_${lang}_CREATE_ASSEMBLY_SOURCE     "<CMAKE_${lang}_COMPILER> <DEFINES> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
endmacro()
