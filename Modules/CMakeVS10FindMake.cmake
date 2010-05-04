
#=============================================================================
# Copyright 2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

# We use MSBuild as the build tool for VS 10
FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES MSBuild
  HINTS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Setup\\VS;ProductDir]
  "$ENV{SYSTEMROOT}/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;CLR Version]/"
  "c:/WINDOWS/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0;CLR Version]/"
  "$ENV{SYSTEMROOT}/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\10.0;CLR Version]/"
  )

MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC10 1)
SET(MSVC_VERSION 1600)

