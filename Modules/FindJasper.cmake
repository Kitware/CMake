# - Try to find the Jasper JPEG2000 library
# Once done this will define
#
#  JASPER_FOUND - system has Jasper
#  JASPER_INCLUDE_DIR - the Jasper include directory
#  JASPER_LIBRARIES - The libraries needed to use Jasper

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

find_package(JPEG)

find_path(JASPER_INCLUDE_DIR jasper/jasper.h)

find_library(JASPER_LIBRARY NAMES jasper libjasper)

# handle the QUIETLY and REQUIRED arguments and set JASPER_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Jasper DEFAULT_MSG JASPER_LIBRARY JASPER_INCLUDE_DIR JPEG_LIBRARIES)

if(JASPER_FOUND)
   set(JASPER_LIBRARIES ${JASPER_LIBRARY} ${JPEG_LIBRARIES} )
endif(JASPER_FOUND)

mark_as_advanced(JASPER_INCLUDE_DIR JASPER_LIBRARY)
