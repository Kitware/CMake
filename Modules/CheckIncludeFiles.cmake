# - Check if the files can be included
#
# CHECK_INCLUDE_FILES(INCLUDE VARIABLE)
#
#  INCLUDE  - list of files to include
#  VARIABLE - variable to return result
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories

#=============================================================================
# Copyright 2003-2012 Kitware, Inc.
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

MACRO(CHECK_INCLUDE_FILES INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_INCLUDE_FILES_INCLUDE_DIRS "-DINCLUDE_DIRECTORIES=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_INCLUDE_FILES_INCLUDE_DIRS)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    SET(CHECK_INCLUDE_FILES_CONTENT "/* */\n")
    SET(MACRO_CHECK_INCLUDE_FILES_FLAGS ${CMAKE_REQUIRED_FLAGS})
    FOREACH(FILE ${INCLUDE})
      SET(CMAKE_CONFIGURABLE_FILE_CONTENT
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT
      "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n\nint main(){return 0;}\n")
    CONFIGURE_FILE("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFiles.c" @ONLY IMMEDIATE)

    SET(_INCLUDE ${INCLUDE}) # remove empty elements
    IF("${_INCLUDE}" MATCHES "^([^;]+);.+;([^;]+)$")
      LIST(LENGTH _INCLUDE _INCLUDE_LEN)
      SET(_description "${_INCLUDE_LEN} include files ${CMAKE_MATCH_1}, ..., ${CMAKE_MATCH_2}")
    ELSEIF("${_INCLUDE}" MATCHES "^([^;]+);([^;]+)$")
      SET(_description "include files ${CMAKE_MATCH_1}, ${CMAKE_MATCH_2}")
    ELSE()
      SET(_description "include file ${_INCLUDE}")
    ENDIF()

    MESSAGE(STATUS "Looking for ${_description}")
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFiles.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_INCLUDE_FILES_FLAGS}
      "${CHECK_INCLUDE_FILES_INCLUDE_DIRS}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${_description} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if files ${INCLUDE} "
        "exist passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${_description} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have includes ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if files ${INCLUDE} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nSource:\n${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILES)
