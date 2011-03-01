# - Find the PostgreSQL installation.
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # PostgreSQL
# FIND_PACKAGE(PostgreSQL)
# ...
# if( PostgreSQL_FOUND )
#   include_directories(${PostgreSQL_INCLUDE_DIRS})
#   link_directories(${PostgreSQL_LIBRARY_DIRS})
# endif( PostgreSQL_FOUND )
# ...
# Remember to include ${PostgreSQL_LIBRARIES} in the target_link_libraries() statement.
#
#
# In Windows, we make the assumption that, if the PostgreSQL files are installed, the default directory
# will be C:\Program Files\PostgreSQL.
#

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
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

# ----------------------------------------------------------------------------
# History:
# This module is derived from the module originally found in the VTK source tree.
#
# ----------------------------------------------------------------------------
# Note:
# PostgreSQL_ADDITIONAL_VERSIONS is a variable that can be used to set the
# version mumber of the implementation of PostgreSQL.
# In Windows the default installation of PostgreSQL uses that as part of the path.
# E.g C:\Program Files\PostgreSQL\8.4.
# Currently, the following version numbers are known to this module:
# "9.1" "9.0" "8.4" "8.3" "8.2" "8.1" "8.0"
#
# To use this variable just do something like this:
# set(PostgreSQL_ADDITIONAL_VERSIONS "9.2" "8.4.4")
# before calling FIND_PACKAGE(PostgreSQL) in your CMakeLists.txt file.
# This will mean that the versions you set here will be found first in the order
# specified before the default ones are searched.
#
# ----------------------------------------------------------------------------
# You may need to manually set:
#  PostgreSQL_INCLUDE_DIR  - the path to where the PostgreSQL include files are.
#  PostgreSQL_LIBRARY_DIR  - The path to where the PostgreSQL library files are.
# If FindPostgreSQL.cmake cannot find the include files or the library files.
#
# ----------------------------------------------------------------------------
# The following variables are set if PostgreSQL is found:
#  PostgreSQL_FOUND         - Set to true when PostgreSQL is found.
#  PostgreSQL_INCLUDE_DIRS  - Include directories for PostgreSQL
#  PostgreSQL_LIBRARY_DIRS  - Link directories for PostgreSQL libraries
#  PostgreSQL_LIBRARIES     - The PostgreSQL libraries.
#
# ----------------------------------------------------------------------------
# If you have installed PostgreSQL in a non-standard location.
# (Please note that in the following comments, it is assumed that <Your Path>
# points to the root directory of the include directory of PostgreSQL.)
# Then you have three options.
# 1) After CMake runs, set PostgreSQL_INCLUDE_DIR to <Your Path>/include and
#    PostgreSQL_LIBRARY_DIR to wherever the library pq (or libpq in windows) is
# 2) Use CMAKE_INCLUDE_PATH to set a path to <Your Path>/PostgreSQL<-version>. This will allow find_path()
#    to locate PostgreSQL_INCLUDE_DIR by utilizing the PATH_SUFFIXES option. e.g. In your CMakeLists.txt file
#    SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "<Your Path>/include")
# 3) Set an environment variable called ${PostgreSQL_ROOT} that points to the root of where you have
#    installed PostgreSQL, e.g. <Your Path>.
#
# ----------------------------------------------------------------------------

set(PostgreSQL_INCLUDE_PATH_DESCRIPTION "top-level directory containing the PostgreSQL include directories. E.g /usr/local/include/PostgreSQL/8.4 or C:/Program Files/PostgreSQL/8.4/include")
set(PostgreSQL_INCLUDE_DIR_MESSAGE "Set the PostgreSQL_INCLUDE_DIR cmake cache entry to the ${PostgreSQL_INCLUDE_PATH_DESCRIPTION}")
set(PostgreSQL_LIBRARY_PATH_DESCRIPTION "top-level directory containing the PostgreSQL libraries.")
set(PostgreSQL_LIBRARY_DIR_MESSAGE "Set the PostgreSQL_LIBRARY_DIR cmake cache entry to the ${PostgreSQL_LIBRARY_PATH_DESCRIPTION}")
set(PostgreSQL_ROOT_DIR_MESSAGE "Set the PostgreSQL_ROOT system variable to where PostgreSQL is found on the machine E.g C:/Program Files/PostgreSQL/8.4")


set(PostgreSQL_ROOT_DIRECTORIES $ENV{PostgreSQL_ROOT})
if(PostgreSQL_ROOT_DIRECTORIES)
  file(TO_CMAKE_PATH ${PostgreSQL_ROOT_DIRECTORIES} PostgreSQL_ROOT_DIRECTORIES)
endif(PostgreSQL_ROOT_DIRECTORIES)

set(PostgreSQL_KNOWN_VERSIONS ${PostgreSQL_ADDITIONAL_VERSIONS}
    "9.1" "9.0" "8.4" "8.3" "8.2" "8.1" "8.0")

# Define additional search paths for root directories.
if ( WIN32 )
  foreach (suffix ${PostgreSQL_KNOWN_VERSIONS} )
    set(PostgreSQL_ADDITIONAL_SEARCH_PATHS ${PostgreSQL_ADDITIONAL_SEARCH_PATHS} "C:/Program Files/PostgreSQL/${suffix}" )
  endforeach(suffix)
endif( WIN32 )
set( PostgreSQL_ROOT_DIRECTORIES
   ${PostgreSQL_ROOT_DIRECTORIES}
   ${PostgreSQL_ROOT}
   ${PostgreSQL_ADDITIONAL_SEARCH_PATHS}
)

#
# Look for an installation.
#
find_path(PostgreSQL_INCLUDE_DIR
  NAMES libpq-fe.h
  PATHS
   # Look in other places.
   ${PostgreSQL_ROOT_DIRECTORIES}
  PATH_SUFFIXES
    postgresql
    include
  # Help the user find it if we cannot.
  DOC "The ${PostgreSQL_INCLUDE_DIR_MESSAGE}"
)

# The PostgreSQL library.
set (PostgreSQL_LIBRARY_TO_FIND pq)
# Setting some more prefixes for the library
set (PostgreSQL_LIB_PREFIX "")
if ( WIN32 )
  set (PostgreSQL_LIB_PREFIX ${PostgreSQL_LIB_PREFIX} "lib")
  set ( PostgreSQL_LIBRARY_TO_FIND ${PostgreSQL_LIB_PREFIX}${PostgreSQL_LIBRARY_TO_FIND})
endif()

find_library( PostgreSQL_LIBRARY
 NAMES ${PostgreSQL_LIBRARY_TO_FIND}
 PATHS
   ${PostgreSQL_ROOT_DIRECTORIES}
 PATH_SUFFIXES
   lib
)
get_filename_component(PostgreSQL_LIBRARY_DIR ${PostgreSQL_LIBRARY} PATH)

# Did we find anything?
set( PostgreSQL_FOUND 0 )
if ( EXISTS "${PostgreSQL_INCLUDE_DIR}" AND EXISTS "${PostgreSQL_LIBRARY_DIR}" )
  set( PostgreSQL_FOUND 1 )
else ( EXISTS "${PostgreSQL_INCLUDE_DIR}" AND EXISTS "${PostgreSQL_LIBRARY_DIR}" )
  if ( POSTGRES_REQUIRED )
    message( FATAL_ERROR "PostgreSQL is required. ${PostgreSQL_ROOT_DIR_MESSAGE}" )
  endif ( POSTGRES_REQUIRED )
endif (EXISTS "${PostgreSQL_INCLUDE_DIR}" AND EXISTS "${PostgreSQL_LIBRARY_DIR}" )

# Now try to get the include and library path.
if(PostgreSQL_FOUND)

  if(EXISTS "${PostgreSQL_INCLUDE_DIR}")
    set(PostgreSQL_INCLUDE_DIRS
      ${PostgreSQL_INCLUDE_DIR}
    )
  endif(EXISTS "${PostgreSQL_INCLUDE_DIR}")

  if(EXISTS "${PostgreSQL_LIBRARY_DIR}")
    set(PostgreSQL_LIBRARY_DIRS
      ${PostgreSQL_LIBRARY_DIR}
    )
    set(PostgreSQL_LIBRARIES ${PostgreSQL_LIBRARY_TO_FIND})
  endif(EXISTS "${PostgreSQL_LIBRARY_DIR}")

  #message("Final PostgreSQL include dir: ${PostgreSQL_INCLUDE_DIRS}")
  #message("Final PostgreSQL library dir: ${PostgreSQL_LIBRARY_DIRS}")
  #message("Final PostgreSQL libraries:   ${PostgreSQL_LIBRARIES}")
endif(PostgreSQL_FOUND)

if(NOT PostgreSQL_FOUND)
  if(NOT PostgreSQL_FIND_QUIETLY)
    message(STATUS "PostgreSQL was not found. ${PostgreSQL_DIR_MESSAGE}")
  else(NOT PostgreSQL_FIND_QUIETLY)
    if(PostgreSQL_FIND_REQUIRED)
      message(FATAL_ERROR "PostgreSQL was not found. ${PostgreSQL_DIR_MESSAGE}")
    endif(PostgreSQL_FIND_REQUIRED)
  endif(NOT PostgreSQL_FIND_QUIETLY)
endif(NOT PostgreSQL_FOUND)
