
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

find_program(CMAKE_MAKE_PROGRAM
  NAMES msdev
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\6.0\\Setup;VsCommonDir]/MSDev98/Bin
  "c:/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  "c:/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  "/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  )
mark_as_advanced(CMAKE_MAKE_PROGRAM)
set(MSVC60 1)
set(MSVC_VERSION 1200)
