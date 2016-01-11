#.rst:
# FindDCMTK
# ---------
#
# - find DCMTK libraries and applications
# The module defines the following variables:
#
#  DCMTK_INCLUDE_DIRS  - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source (default to /usr)
#
#
# This file is able to find version of DCMTK that do or do not export
# a DCMTKConfig.cmake file.
#
# IMPORTANT: A set of patches has been contributed to DCMTK
# maintainers, as soon as:
#   (1) it has been integrated upstream so that it is available in
#   an official release (for example X.Y.Z),
#   (2) code bases have been updated to build against X.Y.Z
# This file could be removed.
#
# The set of patches is listed here: https://github.com/commontk/DCMTK/compare/79030ba...f461865
#
#
# Waiting for this to happen, build systems will have to be able
# to support different versions of DCMTK.
#
# On any given system, the following combination of DCMTK version could be
# considered:
#
#         |   SYSTEM DCMTK      +      LOCAL DCMTK      |     Supported ?   |
# ---------------------------------------------------------------------------
# Case A  |   NA                +      [ ] DCMTKConfig  |         YES       |
# ------------------------------------------------------|--------------------
# Case B  |   NA                +      [X] DCMTKConfig  |         YES       |
# ------------------------------------------------------|--------------------
# Case C  |   [ ] DCMTKConfig   +      NA               |         YES       |
# ------------------------------------------------------|--------------------
# Case D  |   [X] DCMTKConfig   +      NA               |         YES       |
# ------------------------------------------------------|--------------------
# Case E  |   [ ] DCMTKConfig   +      [ ] DCMTKConfig  |         YES (*)   |
# ------------------------------------------------------|--------------------
# Case F  |   [X] DCMTKConfig   +      [ ] DCMTKConfig  |         NO        |
# ------------------------------------------------------|--------------------
# Case G  |   [ ] DCMTKConfig   +      [X] DCMTKConfig  |         YES       |
# ------------------------------------------------------|--------------------
# Case H  |   [X] DCMTKConfig   +      [X] DCMTKConfig  |         YES       |
# ---------------------------------------------------------------------------
#
#  (*) See Troubleshooting section.
#
# Legend:
#
#   NA ...............: Means that no System or Local DCMTK is available
#
#   [ ] DCMTKConfig ..: Means that the version of DCMTK does NOT export a DCMTKConfig.cmake file.
#
#   [X] DCMTKConfig ..: Means that the version of DCMTK exports a DCMTKConfig.cmake file.
#
#
#
# In CTK commits 52d953 and 74b4b07, the FindDCMTK.cmake module has been updated to be able
# to find both version of DCMTK ([ ] DCMTKConfig and [X] DCMTKConfig).
#
# It is a two steps process:
#
#   * Step 1:  Attempt to find DCMTK version providing a DCMTKConfig.cmake file. This is done
#              with the help of ''
#
#   * Step 2:  If step 1 failed, rely on FindDCMTK.cmake to set DCMTK_* variables details below.
#
#
#
# Troubleshooting:
#
#    What to do if my project find a different version of DCMTK ?
#
#    Remove DCMTK entry from the CMake registry.
#    Search for "CMake user package registry" on http://www.cmake.org/cmake/help/v2.8.9/cmake.html#command:find_package
#
#    Details: As some point, the DCMTK build tree was exported [1][2][3]. Indeed, before the set of
#    patches discussed above has been contributed to upstream DCMTK, an initial implementation of
#    a DCMTK build system exporting its build tree has been implemented and has been tested by some
#    folks. This had the effect of populating the CMake registry.
#
#    Since Step1 does not exclude the CMake registry, when dealing with case E, the incorrect version of
#    DCMTK could be found.
#
#    [1] http://slicer-devel.65872.n3.nabble.com/Packaging-seems-to-work-again-tp4028121p4028134.html
#    [2] https://www.assembla.com/spaces/slicerrt/tickets/244-dcmtk_dir-vs--dcmtkconfig-cmake?comment=267984263#comment:267984263
#    [3] http://www.cmake.org/cmake/help/v2.8.11/cmake.html#command:export
#
#
#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2009-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
# Copyright 2010 Thomas Sondergaard <ts@medical-insight.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

#
# Written for VXL by Amitha Perera.
# Upgraded for GDCM by Mathieu Malaterre.
# Modified for EasyViz by Thomas Sondergaard.
#

set(_dcmtk_dir_description "The directory of DCMTK build or install tree.")

# Ensure that DCMTK_DIR is set to a reasonable default value
# so that DCMTK libraries can be found on a standard Unix distribution.
# It also overwrite the value of DCMTK_DIR after this one has been
# set by a successful discovery of DCMTK by the unpatched FindDCMTK.cmake module
# distributed with CMake (as of 0167cea)
if(NOT DCMTK_DIR OR DCMTK_DIR STREQUAL "/usr/include/dcmtk")
  set(DCMTK_DIR "/usr" CACHE PATH ${_dcmtk_dir_description} FORCE)
endif()

set(_SAVED_DCMTK_DIR ${DCMTK_DIR})

#
# Step1: Attempt to find a version of DCMTK providing a DCMTKConfig.cmake file.
#
message(STATUS "Trying to find DCMTK expecting DCMTKConfig.cmake")
find_package(DCMTK QUIET NO_MODULE)
if(DCMTK_FOUND
    AND NOT "x" STREQUAL "x${DCMTK_LIBRARIES}"
    AND NOT "x" STREQUAL "x${DCMTK_INCLUDE_DIRS}")
  message(STATUS "Trying to find DCMTK expecting DCMTKConfig.cmake - ok")
  return()
else()
  message(STATUS "Trying to find DCMTK expecting DCMTKConfig.cmake - failed")
endif()

message(STATUS "Trying to find DCMTK relying on FindDCMTK.cmake")

# Restore the value reset by the previous call to 'find_package(DCMTK QUIET NO_MODULE)'
set(DCMTK_DIR ${_SAVED_DCMTK_DIR} CACHE PATH ${_dcmtk_dir_description} FORCE)


#
# Step2: Attempt to find a version of DCMTK that does NOT provide a DCMTKConfig.cmake file.
#

# prefer DCMTK_DIR over default system paths like /usr/lib
if(DCMTK_DIR)
  set(CMAKE_PREFIX_PATH ${DCMTK_DIR}/lib ${CMAKE_PREFIX_PATH}) # this is given to FIND_LIBRARY or FIND_PATH
endif()

# Find all libraries, store debug and release separately
foreach(lib
    dcmpstat
    dcmsr
    dcmsign
    dcmtls
    dcmqrdb
    dcmnet
    dcmjpeg
    dcmimage
    dcmimgle
    dcmdata
    oflog
    ofstd
    ijg12
    ijg16
    ijg8
    )

  # Find Release libraries
  find_library(DCMTK_${lib}_LIBRARY_RELEASE
    ${lib}
    PATHS
    ${DCMTK_DIR}/${lib}/libsrc
    ${DCMTK_DIR}/${lib}/libsrc/Release
    ${DCMTK_DIR}/${lib}/Release
    ${DCMTK_DIR}/lib
    ${DCMTK_DIR}/lib/Release
    ${DCMTK_DIR}/dcmjpeg/lib${lib}/Release
    NO_DEFAULT_PATH
    )

  # Find Debug libraries
  find_library(DCMTK_${lib}_LIBRARY_DEBUG
    ${lib}${DCMTK_CMAKE_DEBUG_POSTFIX}
    PATHS
    ${DCMTK_DIR}/${lib}/libsrc
    ${DCMTK_DIR}/${lib}/libsrc/Debug
    ${DCMTK_DIR}/${lib}/Debug
    ${DCMTK_DIR}/lib
    ${DCMTK_DIR}/lib/Debug
    ${DCMTK_DIR}/dcmjpeg/lib${lib}/Debug
    NO_DEFAULT_PATH
    )

  mark_as_advanced(DCMTK_${lib}_LIBRARY_RELEASE)
  mark_as_advanced(DCMTK_${lib}_LIBRARY_DEBUG)

  # Add libraries to variable according to build type
  if(DCMTK_${lib}_LIBRARY_RELEASE)
    list(APPEND DCMTK_LIBRARIES optimized ${DCMTK_${lib}_LIBRARY_RELEASE})
  endif()

  if(DCMTK_${lib}_LIBRARY_DEBUG)
    list(APPEND DCMTK_LIBRARIES debug ${DCMTK_${lib}_LIBRARY_DEBUG})
  endif()

endforeach()

set(CMAKE_THREAD_LIBS_INIT)
if(DCMTK_oflog_LIBRARY_RELEASE OR DCMTK_oflog_LIBRARY_DEBUG)
  # Hack - Not having a DCMTKConfig.cmake file to read the settings from, we will attempt to
  # find the library in all cases.
  # Ideally, pthread library should be discovered only if DCMTK_WITH_THREADS is enabled.
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  find_package(Threads)
endif()

if(CMAKE_THREAD_LIBS_INIT)
  list(APPEND DCMTK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

#
# SPECIFIC CASE FOR DCMTK BUILD DIR as DCMTK_DIR
# (as opposed to a DCMTK install dir)
# Have to find the source directory.
if(EXISTS ${DCMTK_DIR}/CMakeCache.txt)
          load_cache(${DCMTK_DIR} READ_WITH_PREFIX "EXT"
          DCMTK_SOURCE_DIR)
  if(NOT EXISTS ${EXTDCMTK_SOURCE_DIR})
    message(FATAL_ERROR
      "DCMTK build directory references
nonexistant DCMTK source directory ${EXTDCMTK_SOURCE_DIR}")
  endif()
endif()

set(DCMTK_config_TEST_HEADER osconfig.h)
set(DCMTK_dcmdata_TEST_HEADER dctypes.h)
set(DCMTK_dcmimage_TEST_HEADER dicoimg.h)
set(DCMTK_dcmimgle_TEST_HEADER dcmimage.h)
set(DCMTK_dcmjpeg_TEST_HEADER djdecode.h)
set(DCMTK_dcmnet_TEST_HEADER assoc.h)
set(DCMTK_dcmpstat_TEST_HEADER dcmpstat.h)
set(DCMTK_dcmqrdb_TEST_HEADER dcmqrdba.h)
set(DCMTK_dcmsign_TEST_HEADER sicert.h)
set(DCMTK_dcmsr_TEST_HEADER dsrtree.h)
set(DCMTK_dcmtls_TEST_HEADER tlslayer.h)
set(DCMTK_ofstd_TEST_HEADER ofstdinc.h)
set(DCMTK_oflog_TEST_HEADER oflog.h)
set(DCMTK_dcmjpls_TEST_HEADER djlsutil.h)

set(DCMTK_INCLUDE_DIR_NAMES)

foreach(dir
    config
    dcmdata
    dcmimage
    dcmimgle
    dcmjpeg
    dcmjpls
    dcmnet
    dcmpstat
    dcmqrdb
    dcmsign
    dcmsr
    dcmtls
    ofstd
    oflog)
  if(EXTDCMTK_SOURCE_DIR)
    set(SOURCE_DIR_PATH
      ${EXTDCMTK_SOURCE_DIR}/${dir}/include/dcmtk/${dir})
  endif()
  find_path(DCMTK_${dir}_INCLUDE_DIR
    ${DCMTK_${dir}_TEST_HEADER}
    PATHS
    ${DCMTK_DIR}/${dir}/include
    ${DCMTK_DIR}/${dir}
    ${DCMTK_DIR}/include/dcmtk/${dir}
    ${DCMTK_DIR}/${dir}/include/dcmtk/${dir}
    ${DCMTK_DIR}/include/${dir}
    ${SOURCE_DIR_PATH}
    )
  mark_as_advanced(DCMTK_${dir}_INCLUDE_DIR)
  list(APPEND DCMTK_INCLUDE_DIR_NAMES DCMTK_${dir}_INCLUDE_DIR)

  if(DCMTK_${dir}_INCLUDE_DIR)
    # add the 'include' path so eg
    #include "dcmtk/dcmimgle/dcmimage.h"
    # works
    get_filename_component(_include ${DCMTK_${dir}_INCLUDE_DIR} PATH)
    get_filename_component(_include ${_include} PATH)
    list(APPEND
      DCMTK_INCLUDE_DIRS
      ${DCMTK_${dir}_INCLUDE_DIR}
      ${_include})
  endif()
endforeach()

list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_DIR}/include)

if(WIN32)
  list(APPEND DCMTK_LIBRARIES netapi32 wsock32)
endif()

if(DCMTK_ofstd_INCLUDE_DIR)
  get_filename_component(DCMTK_dcmtk_INCLUDE_DIR
    ${DCMTK_ofstd_INCLUDE_DIR}
    PATH
    CACHE)
  list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_dcmtk_INCLUDE_DIR})
  mark_as_advanced(DCMTK_dcmtk_INCLUDE_DIR)
endif()

# Compatibility: This variable is deprecated
set(DCMTK_INCLUDE_DIR ${DCMTK_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCMTK
  REQUIRED_VARS ${DCMTK_INCLUDE_DIR_NAMES} DCMTK_LIBRARIES
  FAIL_MESSAGE "Please set DCMTK_DIR and re-run configure")

# Workaround bug in packaging of DCMTK 3.6.0 on Debian.
# See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=637687
if(DCMTK_FOUND AND UNIX AND NOT APPLE)
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS )
  set(CMAKE_REQUIRED_DEFINITIONS )
  set(CMAKE_REQUIRED_INCLUDES ${DCMTK_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${DCMTK_LIBRARIES})
  check_cxx_source_compiles("#include <dcmtk/config/osconfig.h>\n#include <dcmtk/ofstd/ofstream.h>\nint main(int,char*[]){return 0;}"
    DCMTK_HAVE_CONFIG_H_OPTIONAL
    )
  if(NOT DCMTK_HAVE_CONFIG_H_OPTIONAL)
    set(DCMTK_DEFINITIONS "HAVE_CONFIG_H")
  endif()
endif()

message(STATUS "Trying to find DCMTK relying on FindDCMTK.cmake - ok")
