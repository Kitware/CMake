# - Try to find the Cups printing system
# Once done this will define
#
#  CUPS_FOUND - system has Cups
#  CUPS_INCLUDE_DIR - the Cups include directory
#  CUPS_LIBRARIES - Libraries needed to use Cups
#  CUPS_VERSION_STRING - version of Cups found (since CMake 2.8.8)
#  Set CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE to TRUE if you need a version which
#  features this function (i.e. at least 1.1.19)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
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

find_path(CUPS_INCLUDE_DIR cups/cups.h )

find_library(CUPS_LIBRARIES NAMES cups )

if (CUPS_INCLUDE_DIR AND CUPS_LIBRARIES AND CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE)
    include(CheckLibraryExists)

    # ippDeleteAttribute is new in cups-1.1.19 (and used by kdeprint)
    CHECK_LIBRARY_EXISTS(cups ippDeleteAttribute "" CUPS_HAS_IPP_DELETE_ATTRIBUTE)
endif (CUPS_INCLUDE_DIR AND CUPS_LIBRARIES AND CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE)

if (CUPS_INCLUDE_DIR AND EXISTS "${CUPS_INCLUDE_DIR}/cups/cups.h")
    file(STRINGS "${CUPS_INCLUDE_DIR}/cups/cups.h" cups_version_str
         REGEX "^#[\t ]*define[\t ]+CUPS_VERSION_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

    unset(CUPS_VERSION_STRING)
    foreach(VPART MAJOR MINOR PATCH)
        foreach(VLINE ${cups_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+CUPS_VERSION_${VPART}")
                string(REGEX REPLACE "^#[\t ]*define[\t ]+CUPS_VERSION_${VPART}[\t ]+([0-9]+)$" "\\1"
                       CUPS_VERSION_PART "${VLINE}")
                if(CUPS_VERSION_STRING)
                    set(CUPS_VERSION_STRING "${CUPS_VERSION_STRING}.${CUPS_VERSION_PART}")
                else(CUPS_VERSION_STRING)
                    set(CUPS_VERSION_STRING "${CUPS_VERSION_PART}")
                endif(CUPS_VERSION_STRING)
            endif()
        endforeach(VLINE)
    endforeach(VPART)
endif (CUPS_INCLUDE_DIR AND EXISTS "${CUPS_INCLUDE_DIR}/cups/cups.h")

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

if (CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cups
                                      REQUIRED_VARS CUPS_LIBRARIES CUPS_INCLUDE_DIR CUPS_HAS_IPP_DELETE_ATTRIBUTE
                                      VERSION_VAR CUPS_VERSION_STRING)
else (CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cups
                                      REQUIRED_VARS CUPS_LIBRARIES CUPS_INCLUDE_DIR
                                      VERSION_VAR CUPS_VERSION_STRING)
endif (CUPS_REQUIRE_IPP_DELETE_ATTRIBUTE)

mark_as_advanced(CUPS_INCLUDE_DIR CUPS_LIBRARIES)
