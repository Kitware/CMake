# - Find SWIG
# This module finds an installed SWIG.  It sets the following variables:
#  SWIG_FOUND - set to true if SWIG is found
#  SWIG_DIR - the directory where swig is installed
#  SWIG_EXECUTABLE - the path to the swig executable
#  SWIG_VERSION   - the version number of the swig executable
#
# The minimum required version of SWIG can be specified using the
# standard syntax, e.g. FIND_PACKAGE(SWIG 1.1)
#
# All information is collected from the SWIG_EXECUTABLE so the
# version to be found can be changed from the command line by
# means of setting SWIG_EXECUTABLE
#

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
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

FIND_PROGRAM(SWIG_EXECUTABLE NAMES swig2.0 swig)

IF(SWIG_EXECUTABLE)
  EXECUTE_PROCESS(COMMAND ${SWIG_EXECUTABLE} -swiglib
    OUTPUT_VARIABLE SWIG_swiglib_output
    ERROR_VARIABLE SWIG_swiglib_error
    RESULT_VARIABLE SWIG_swiglib_result)

  IF(SWIG_swiglib_result) 
    IF(SWIG_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Command \"${SWIG_EXECUTABLE} -swiglib\" failed with output:\n${SWIG_swiglib_error}")
    ELSE(SWIG_FIND_REQUIRED)
      MESSAGE(STATUS "Command \"${SWIG_EXECUTABLE} -swiglib\" failed with output:\n${SWIG_swiglib_error}")
    ENDIF(SWIG_FIND_REQUIRED)
  ELSE(SWIG_swiglib_result)
    STRING(REGEX REPLACE "[\n\r]+" ";" SWIG_swiglib_output ${SWIG_swiglib_output})
    # force the path to be computed each time in case SWIG_EXECUTABLE has changed.
    SET(SWIG_DIR SWIG_DIR-NOTFOUND)
    FIND_PATH(SWIG_DIR swig.swg PATHS ${SWIG_swiglib_output})
    IF(SWIG_DIR)
      SET(SWIG_USE_FILE ${CMAKE_ROOT}/Modules/UseSWIG.cmake)
      EXECUTE_PROCESS(COMMAND ${SWIG_EXECUTABLE} -version
        OUTPUT_VARIABLE SWIG_version_output
        ERROR_VARIABLE SWIG_version_output
        RESULT_VARIABLE SWIG_version_result)
      IF(SWIG_version_result)
        MESSAGE(SEND_ERROR "Command \"${SWIG_EXECUTABLE} -version\" failed with output:\n${SWIG_version_output}")
      ELSE(SWIG_version_result)
        STRING(REGEX REPLACE ".*SWIG Version[^0-9.]*\([0-9.]+\).*" "\\1"
          SWIG_version_output "${SWIG_version_output}")
        SET(SWIG_VERSION ${SWIG_version_output} CACHE STRING "Swig version" FORCE)
      ENDIF(SWIG_version_result)
    ENDIF(SWIG_DIR)
  ENDIF(SWIG_swiglib_result)
ENDIF(SWIG_EXECUTABLE)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SWIG  REQUIRED_VARS SWIG_EXECUTABLE SWIG_DIR
                                        VERSION_VAR SWIG_VERSION )
