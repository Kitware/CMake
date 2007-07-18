# - Try to find the Cups printing system
# Once done this will define
#
#  CUPS_FOUND - system has Cups
#  CUPS_INCLUDE_DIR - the Cups include directory
#  CUPS_LIBRARIES - Libraries needed to use Cups
#  Set CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE to TRUE if you need a version which 
#  features this function (i.e. at least 1.1.19)

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


INCLUDE(CheckLibraryExists)

FIND_PATH(CUPS_INCLUDE_DIR cups/cups.h )

FIND_LIBRARY(CUPS_LIBRARIES NAMES cups )

IF (CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)
   SET(CUPS_FOUND TRUE)

   # ippDeleteAttribute is new in cups-1.1.19 (and used by kdeprint)
   CHECK_LIBRARY_EXISTS(cups ippDeleteAttribute "" CUPS_HAS_IPP_DELETE_ATTRIBUTE)
   IF (CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE AND NOT CUPS_HAS_IPP_DELETE_ATTRIBUTE)
      SET(CUPS_FOUND FALSE)
   ENDIF (CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE AND NOT CUPS_HAS_IPP_DELETE_ATTRIBUTE)

ELSE  (CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)
   SET(CUPS_FOUND FALSE)
ENDIF (CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)

IF (CUPS_FOUND)
   IF (NOT Cups_FIND_QUIETLY)
      MESSAGE(STATUS "Found Cups: ${CUPS_LIBRARIES}")
   ENDIF (NOT Cups_FIND_QUIETLY)
ELSE (CUPS_FOUND)
   SET(CUPS_LIBRARIES )
   IF (Cups_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find Cups")
   ENDIF (Cups_FIND_REQUIRED)
ENDIF (CUPS_FOUND)
  
  
MARK_AS_ADVANCED(CUPS_INCLUDE_DIR CUPS_LIBRARIES)
  
