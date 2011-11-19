
#=============================================================================
# Copyright 2009 Kitware, Inc.
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

# This file is included in CMakeSystemSpecificInformation.cmake if
# the Eclipse CDT4 extra generator has been selected.

FIND_PROGRAM(CMAKE_ECLIPSE_EXECUTABLE NAMES eclipse DOC "The Eclipse executable")

FUNCTION(_FIND_ECLIPSE_VERSION)
  # This code is in a function so the variables used here have only local scope
  IF(CMAKE_ECLIPSE_EXECUTABLE)
    GET_FILENAME_COMPONENT(_ECLIPSE_DIR "${CMAKE_ECLIPSE_EXECUTABLE}" PATH)
    FILE(GLOB _ECLIPSE_FEATURE_DIR "${_ECLIPSE_DIR}/features/org.eclipse.platform*")
    IF("${_ECLIPSE_FEATURE_DIR}" MATCHES ".+org.eclipse.platform_([0-9]+\\.[0-9]+).+")
      SET(_ECLIPSE_VERSION ${CMAKE_MATCH_1})
    ENDIF()
  ENDIF()

  # Set up a map with the names of the Eclipse releases:
  SET(_ECLIPSE_VERSION_NAME_    "Unknown" )
  SET(_ECLIPSE_VERSION_NAME_3.2 "Callisto" )
  SET(_ECLIPSE_VERSION_NAME_3.3 "Europa" )
  SET(_ECLIPSE_VERSION_NAME_3.4 "Ganymede" )
  SET(_ECLIPSE_VERSION_NAME_3.5 "Galileo" )
  SET(_ECLIPSE_VERSION_NAME_3.6 "Helios" )
  SET(_ECLIPSE_VERSION_NAME_3.7 "Indigo" )

  IF(_ECLIPSE_VERSION)
    MESSAGE(STATUS "Found Eclipse version ${_ECLIPSE_VERSION} (${_ECLIPSE_VERSION_NAME_${_ECLIPSE_VERSION}})")
  ELSE()
    SET(_ECLIPSE_VERSION "3.6" )
    MESSAGE(STATUS "Could not determine Eclipse version, assuming at least ${_ECLIPSE_VERSION} (${_ECLIPSE_VERSION_NAME_${_ECLIPSE_VERSION}}). Adjust CMAKE_ECLIPSE_VERSION if this is wrong.")
  ENDIF()

  SET(CMAKE_ECLIPSE_VERSION "${_ECLIPSE_VERSION} (${_ECLIPSE_VERSION_NAME_${_ECLIPSE_VERSION}})" CACHE STRING "The version of Eclipse. If Eclipse has not been found, 3.6 (Helios) is assumed.")
  SET_PROPERTY(CACHE CMAKE_ECLIPSE_VERSION PROPERTY STRINGS "3.2 (${_ECLIPSE_VERSION_NAME_3.2})"
                                                            "3.3 (${_ECLIPSE_VERSION_NAME_3.3})"
                                                            "3.4 (${_ECLIPSE_VERSION_NAME_3.4})"
                                                            "3.5 (${_ECLIPSE_VERSION_NAME_3.5})"
                                                            "3.6 (${_ECLIPSE_VERSION_NAME_3.6})"
                                                            "3.7 (${_ECLIPSE_VERSION_NAME_3.7})")
ENDFUNCTION()

_FIND_ECLIPSE_VERSION()

# Try to find out how many CPUs we have and set the -j argument for make accordingly
SET(_CMAKE_ECLIPSE_INITIAL_MAKE_ARGS "")

INCLUDE(ProcessorCount)
PROCESSORCOUNT(_CMAKE_ECLIPSE_PROCESSOR_COUNT)

# Only set -j if we are under UNIX and if the make-tool used actually has "make" in the name
# (we may also get here in the future e.g. for ninja)
IF("${_CMAKE_ECLIPSE_PROCESSOR_COUNT}" GREATER 1  AND  UNIX  AND  "${CMAKE_MAKE_PROGRAM}" MATCHES make)
  SET(_CMAKE_ECLIPSE_INITIAL_MAKE_ARGS "-j${_CMAKE_ECLIPSE_PROCESSOR_COUNT}")
ENDIF()

# This variable is used by the Eclipse generator and appended to the make invocation commands.
SET(CMAKE_ECLIPSE_MAKE_ARGUMENTS "${_CMAKE_ECLIPSE_INITIAL_MAKE_ARGS}" CACHE STRING "Additional command line arguments when Eclipse invokes make. Enter e.g. -j<some_number> to get parallel builds")

# This variable is used by the Eclipse generator in out-of-source builds only.
SET(CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT FALSE CACHE BOOL "If enabled, CMake will generate a source project for Eclipse in CMAKE_SOURCE_DIR")
MARK_AS_ADVANCED(CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT)

# Determine builtin macros and include dirs:
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/CMakeExtraGeneratorDetermineCompilerMacrosAndIncludeDirs.cmake)
