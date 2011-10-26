# - Try to find the Cups printing system
# Once done this will define
#
#  CUPS_FOUND - system has Cups
#  CUPS_INCLUDE_DIR - the Cups include directory
#  CUPS_LIBRARIES - Libraries needed to use Cups
#  Set CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE to TRUE if you need a version which
#  features this function (i.e. at least 1.1.19)

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

include(CheckLibraryExists)

find_path(CUPS_INCLUDE_DIR cups/cups.h )

find_library(CUPS_LIBRARIES NAMES cups )

if(CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)
   set(CUPS_FOUND TRUE)

   # ippDeleteAttribute is new in cups-1.1.19 (and used by kdeprint)
   CHECK_LIBRARY_EXISTS(cups ippDeleteAttribute "" CUPS_HAS_IPP_DELETE_ATTRIBUTE)
   if(CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE AND NOT CUPS_HAS_IPP_DELETE_ATTRIBUTE)
      set(CUPS_FOUND FALSE)
   endif(CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE AND NOT CUPS_HAS_IPP_DELETE_ATTRIBUTE)

else(CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)
   set(CUPS_FOUND FALSE)
endif(CUPS_INCLUDE_DIR AND CUPS_LIBRARIES)

if(CUPS_FOUND)
   if(NOT Cups_FIND_QUIETLY)
      message(STATUS "Found Cups: ${CUPS_LIBRARIES}")
   endif(NOT Cups_FIND_QUIETLY)
else(CUPS_FOUND)
   set(CUPS_LIBRARIES )
   if(Cups_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find Cups")
   endif(Cups_FIND_REQUIRED)
endif(CUPS_FOUND)


mark_as_advanced(CUPS_INCLUDE_DIR CUPS_LIBRARIES)

