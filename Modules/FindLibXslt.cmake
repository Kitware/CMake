# - Try to find the LibXslt library
# Once done this will define
#
#  LIBXSLT_FOUND - system has LibXslt
#  LIBXSLT_INCLUDE_DIR - the LibXslt include directory
#  LIBXSLT_LIBRARIES - Link these to LibXslt
#  LIBXSLT_DEFINITIONS - Compiler switches required for using LibXslt
# Additionally, the following two variables are set (but not required for using xslt):
#  LIBXSLT_EXSLT_LIBRARIES - Link to these if you need to link against the exslt library
#  LIBXSLT_XSLTPROC_EXECUTABLE - Contains the full path to the xsltproc executable if found

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
# in the FIND_PATH() and FIND_LIBRARY() calls
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_LIBXSLT QUIET libxslt)
SET(LIBXSLT_DEFINITIONS ${PC_LIBXSLT_CFLAGS_OTHER})

FIND_PATH(LIBXSLT_INCLUDE_DIR NAMES libxslt/xslt.h
    HINTS
   ${PC_LIBXSLT_INCLUDEDIR}
   ${PC_LIBXSLT_INCLUDE_DIRS}
  )

FIND_LIBRARY(LIBXSLT_LIBRARIES NAMES xslt libxslt
    HINTS
   ${PC_LIBXSLT_LIBDIR}
   ${PC_LIBXSLT_LIBRARY_DIRS}
  )

FIND_LIBRARY(LIBXSLT_EXSLT_LIBRARY NAMES exslt libexslt
    HINTS
    ${PC_LIBXSLT_LIBDIR}
    ${PC_LIBXSLT_LIBRARY_DIRS}
  )

SET(LIBXSLT_EXSLT_LIBRARIES ${LIBXSLT_EXSLT_LIBRARY} )

FIND_PROGRAM(LIBXSLT_XSLTPROC_EXECUTABLE xsltproc)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXslt DEFAULT_MSG LIBXSLT_LIBRARIES LIBXSLT_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBXSLT_INCLUDE_DIR
                 LIBXSLT_LIBRARIES
                 LIBXSLT_EXSLT_LIBRARY
                 LIBXSLT_XSLTPROC_EXECUTABLE)
