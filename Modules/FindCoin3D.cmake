# - Find Coin3D (Open Inventor)
# Coin3D is an implementation of the Open Inventor API.
# It provides data structures and algorithms for 3D visualization
# http://www.coin3d.org/
#
# This module defines the following variables
#  COIN3D_FOUND         - system has Coin3D - Open Inventor
#  COIN3D_INCLUDE_DIRS  - where the Inventor include directory can be found
#  COIN3D_LIBRARIES     - Link to this to use Coin3D
#

#=============================================================================
# Copyright 2008-2009 Kitware, Inc.
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

if(WIN32)
  if(CYGWIN)

    find_path(COIN3D_INCLUDE_DIRS Inventor/So.h)
    find_library(COIN3D_LIBRARIES Coin)

  else(CYGWIN)

    find_path(COIN3D_INCLUDE_DIRS Inventor/So.h
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SIM\\Coin3D\\2;Installation Path]/include"
    )

    find_library(COIN3D_LIBRARY_DEBUG coin2d
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SIM\\Coin3D\\2;Installation Path]/lib"
    )

    find_library(COIN3D_LIBRARY_RELEASE coin2
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\SIM\\Coin3D\\2;Installation Path]/lib"
    )

    if(COIN3D_LIBRARY_DEBUG AND COIN3D_LIBRARY_RELEASE)
      set(COIN3D_LIBRARIES optimized ${COIN3D_LIBRARY_RELEASE}
                           debug ${COIN3D_LIBRARY_DEBUG})
    else(COIN3D_LIBRARY_DEBUG AND COIN3D_LIBRARY_RELEASE)
      if(COIN3D_LIBRARY_DEBUG)
        set(COIN3D_LIBRARIES ${COIN3D_LIBRARY_DEBUG})
      endif(COIN3D_LIBRARY_DEBUG)
      if(COIN3D_LIBRARY_RELEASE)
        set(COIN3D_LIBRARIES ${COIN3D_LIBRARY_RELEASE})
      endif(COIN3D_LIBRARY_RELEASE)
    endif(COIN3D_LIBRARY_DEBUG AND COIN3D_LIBRARY_RELEASE)

  endif(CYGWIN)

else(WIN32)
  if(APPLE)
    find_path(COIN3D_INCLUDE_DIRS Inventor/So.h
     /Library/Frameworks/Inventor.framework/Headers
    )
    find_library(COIN3D_LIBRARIES Coin
      /Library/Frameworks/Inventor.framework/Libraries
    )
    set(COIN3D_LIBRARIES "-framework Coin3d" CACHE STRING "Coin3D library for OSX")
  else(APPLE)

    find_path(COIN3D_INCLUDE_DIRS Inventor/So.h)
    find_library(COIN3D_LIBRARIES Coin)

  endif(APPLE)

endif(WIN32)

# handle the QUIETLY and REQUIRED arguments and set COIN3D_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Coin3D DEFAULT_MSG COIN3D_LIBRARIES COIN3D_INCLUDE_DIRS)

mark_as_advanced(COIN3D_INCLUDE_DIRS COIN3D_LIBRARIES )


