
#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
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

# VCExpress does not support cross compiling, which is necessary for Win CE
set( _CMAKE_MAKE_PROGRAM_NAMES devenv)
if(NOT CMAKE_CROSSCOMPILING)
  set( _CMAKE_MAKE_PROGRAM_NAMES ${_CMAKE_MAKE_PROGRAM_NAMES} VCExpress)
endif(NOT CMAKE_CROSSCOMPILING)

find_program(CMAKE_MAKE_PROGRAM
  NAMES ${_CMAKE_MAKE_PROGRAM_NAMES}
  HINTS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VS;EnvironmentDirectory]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup;Dbghelp_path]
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio8/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio 8/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio8/Common7/IDE"
  "/Program Files/Microsoft Visual Studio 8/Common7/IDE/"
  )
mark_as_advanced(CMAKE_MAKE_PROGRAM)
set(MSVC80 1)
set(MSVC_VERSION 1400)
