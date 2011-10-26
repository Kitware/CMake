# - Try to find BZip2
# Once done this will define
#
#  BZIP2_FOUND - system has BZip2
#  BZIP2_INCLUDE_DIR - the BZip2 include directory
#  BZIP2_LIBRARIES - Link these to use BZip2
#  BZIP2_NEED_PREFIX - this is set if the functions are prefixed with BZ2_

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

find_path(BZIP2_INCLUDE_DIR bzlib.h )

find_library(BZIP2_LIBRARIES NAMES bz2 bzip2 )

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BZip2 DEFAULT_MSG BZIP2_LIBRARIES BZIP2_INCLUDE_DIR)

if(BZIP2_FOUND)
   include(CheckLibraryExists)
   CHECK_LIBRARY_EXISTS(${BZIP2_LIBRARIES} BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
endif(BZIP2_FOUND)

mark_as_advanced(BZIP2_INCLUDE_DIR BZIP2_LIBRARIES)

