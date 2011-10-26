# - Try to find the LibXslt library
# Once done this will define
#
#  LIBXSLT_FOUND - system has LibXslt
#  LIBXSLT_INCLUDE_DIR - the LibXslt include directory
#  LIBXSLT_LIBRARIES - Link these to LibXslt
#  LIBXSLT_DEFINITIONS - Compiler switches required for using LibXslt

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
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

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig)
PKG_CHECK_MODULES(PC_LIBXSLT libxslt)
set(LIBXSLT_DEFINITIONS ${PC_LIBXSLT_CFLAGS_OTHER})

find_path(LIBXSLT_INCLUDE_DIR NAMES libxslt/xslt.h
    HINTS
   ${PC_LIBXSLT_INCLUDEDIR}
   ${PC_LIBXSLT_INCLUDE_DIRS}
  )

find_library(LIBXSLT_LIBRARIES NAMES xslt libxslt
    HINTS
   ${PC_LIBXSLT_LIBDIR}
   ${PC_LIBXSLT_LIBRARY_DIRS}
  )

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXslt DEFAULT_MSG LIBXSLT_LIBRARIES LIBXSLT_INCLUDE_DIR)

mark_as_advanced(LIBXSLT_INCLUDE_DIR LIBXSLT_LIBRARIES)

