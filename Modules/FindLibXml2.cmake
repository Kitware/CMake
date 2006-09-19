# - Try to find LibXml2
# Once done this will define
#
#  LIBXML2_FOUND - system has LibXml2
#  LIBXML2_INCLUDE_DIR - the LibXml2 include directory
#  LIBXML2_LIBRARIES - the libraries needed to use LibXml2
#  LIBXML2_DEFINITIONS - Compiler switches required for using LibXml2
#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# This code is available under the BSD license, see licenses/BSD for details.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)
   # in cache already
   SET(LibXml2_FIND_QUIETLY TRUE)
ENDIF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
   PKGCONFIG(libxml-2.0 _LibXml2IncDir _LibXml2LinkDir _LibXml2LinkFlags _LibXml2Cflags)
   SET(LIBXML2_DEFINITIONS ${_LibXml2Cflags})
ENDIF (NOT WIN32)

FIND_PATH(LIBXML2_INCLUDE_DIR libxml/xpath.h
   PATHS
   ${_LibXml2IncDir}
   PATH_SUFFIXES libxml2
   )

FIND_LIBRARY(LIBXML2_LIBRARIES NAMES xml2 libxml2
   PATHS
   ${_LibXml2LinkDir}
   )

IF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)
   SET(LIBXML2_FOUND TRUE)
ELSE (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)
   SET(LIBXML2_FOUND FALSE)
ENDIF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)

IF (LIBXML2_FOUND)
   IF (NOT LibXml2_FIND_QUIETLY)
      MESSAGE(STATUS "Found LibXml2: ${LIBXML2_LIBRARIES}")
   ENDIF (NOT LibXml2_FIND_QUIETLY)
ELSE (LIBXML2_FOUND)
   IF (LibXml2_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find LibXml2")
   ENDIF (LibXml2_FIND_REQUIRED)
ENDIF (LIBXML2_FOUND)

MARK_AS_ADVANCED(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES)

