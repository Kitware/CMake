
#=============================================================================
# Copyright 2007-2011 Kitware, Inc.
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

# Look for devenv as a build program.  We need to use this to support
# Intel Fortran integration into VS.  MSBuild can not be used for that case
# since Intel Fortran uses the older devenv file format.
FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES devenv
  HINTS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup\\VS;EnvironmentDirectory]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup;Dbghelp_path]
  "$ENV{ProgramFiles}/Microsoft Visual Studio 11.0/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio11.0/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 11/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio11/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio 11.0/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio11.0/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio 11/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio11/Common7/IDE"
  "/Program Files/Microsoft Visual Studio 11.0/Common7/IDE/"
  "/Program Files/Microsoft Visual Studio 11/Common7/IDE/"
  )

# if devenv is not found, then use MSBuild.
# it is expected that if devenv is not found, then we are
# dealing with Visual Studio Express.  VCExpress has random
# failures when being run as a command line build tool which
# causes the compiler checks and try-compile stuff to fail. MSbuild
# is a better choice for this.  However, VCExpress does not support
# cross compiling needed for Win CE.
IF(NOT CMAKE_CROSSCOMPILING)
  FIND_PROGRAM(CMAKE_MAKE_PROGRAM
    NAMES MSBuild
    HINTS
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup\\VS;ProductDir]
    "$ENV{SYSTEMROOT}/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0;CLR Version]/"
    "c:/WINDOWS/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0;CLR Version]/"
    "$ENV{SYSTEMROOT}/Microsoft.NET/Framework/[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\11.0;CLR Version]/")
ENDIF()

MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC11 1)
SET(MSVC_VERSION 1700)
