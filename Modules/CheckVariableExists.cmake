# - Check if the variable exists.
#  CHECK_VARIABLE_EXISTS(VAR VARIABLE)
#
#  VAR      - the name of the variable
#  VARIABLE - variable to store the result
#
# This macro is only for C variables.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

INCLUDE("${CMAKE_CURRENT_LIST_DIR}/CMakeExpandImportedTargets.cmake")


MACRO(CHECK_VARIABLE_EXISTS VAR VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_VARIABLE_DEFINITIONS
      "-DCHECK_VARIABLE_EXISTS=${VAR} ${CMAKE_REQUIRED_FLAGS}")
    MESSAGE(STATUS "Looking for ${VAR}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      # this one translates potentially used imported library targets to their files on disk
      CMAKE_EXPAND_IMPORTED_TARGETS(_ADJUSTED_CMAKE_REQUIRED_LIBRARIES  LIBRARIES  ${CMAKE_REQUIRED_LIBRARIES} CONFIGURATION "${CMAKE_TRY_COMPILE_CONFIGURATION}")
      SET(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${_ADJUSTED_CMAKE_REQUIRED_LIBRARIES}")
    ELSE(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES)
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/CheckVariableExists.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_VARIABLE_DEFINITIONS}
      "${CHECK_VARIABLE_EXISTS_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      SET(${VARIABLE} 1 CACHE INTERNAL "Have variable ${VAR}")
      MESSAGE(STATUS "Looking for ${VAR} - found")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the variable ${VAR} exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      SET(${VARIABLE} "" CACHE INTERNAL "Have variable ${VAR}")
      MESSAGE(STATUS "Looking for ${VAR} - not found")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the variable ${VAR} exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_VARIABLE_EXISTS)
