
#=============================================================================
# Copyright 2016 Kitware, Inc.
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
if(__SUNOS_COMPILER_PATHSCALE)
  return()
endif()
set(__SUNOS_COMPILER_PATHSCALE 1)

macro(__sunos_compiler_pathscale lang)
  # Shared library compile and link flags.
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "-fPIC")
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIE "-fPIE")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-fPIC")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared")

  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,-R")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":")
  set(CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,-h")
endmacro()
