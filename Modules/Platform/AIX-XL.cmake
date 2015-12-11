
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
if(__AIX_COMPILER_XL)
  return()
endif()
set(__AIX_COMPILER_XL 1)

#
# By default, runtime linking is enabled. All shared objects specified on the command line
# will be listed, even if there are no symbols referenced, in the output file.
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,-brtl")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-Wl,-brtl")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-brtl")


macro(__aix_compiler_xl lang)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,-blibpath:")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-G -Wl,-bnoipath")  # -shared
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-Wl,-bexpall")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS " ")
  set(CMAKE_SHARED_MODULE_${lang}_FLAGS  " ")

  set(CMAKE_${lang}_LINK_FLAGS "-Wl,-bnoipath")
endmacro()
