
#=============================================================================
# Copyright 2007-2013 Kitware, Inc.
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

# Always use MSBuild because:
# - devenv treats command-line builds as recently-loaded projects in the IDE
# - devenv does not appear to support non-standard platform toolsets
# If we need devenv for Intel Fortran in the future we should add
# a special case when Fortran is enabled.
find_program(CMAKE_MAKE_PROGRAM
  NAMES MSBuild
  HINTS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\12.0;MSBuildToolsPath]"
  )

mark_as_advanced(CMAKE_MAKE_PROGRAM)
set(MSVC12 1)
set(MSVC_VERSION 1800)
