
#=============================================================================
# Copyright 2002-2012 Kitware, Inc.
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
if(__COMPILER_CLANG)
  return()
endif()
set(__COMPILER_CLANG 1)

include(Compiler/GNU)

macro(__compiler_clang lang)
  __compiler_gnu(${lang})
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "-fPIE")
  set(CMAKE_INCLUDE_SYSTEM_FLAG_${lang} "-isystem ")
endmacro()
