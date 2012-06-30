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

include("${CMAKE_CURRENT_LIST_DIR}/CMakeExpandImportedTargets.cmake")


macro(CHECK_VARIABLE_EXISTS VAR VARIABLE)
  if("${VARIABLE}" MATCHES "^${VARIABLE}$")
    set(MACRO_CHECK_VARIABLE_DEFINITIONS
      "-DCHECK_VARIABLE_EXISTS=${VAR} ${CMAKE_REQUIRED_FLAGS}")
    message(STATUS "Looking for ${VAR}")
    if(CMAKE_REQUIRED_LIBRARIES)
      # this one translates potentially used imported library targets to their files on disk
      CMAKE_EXPAND_IMPORTED_TARGETS(_ADJUSTED_CMAKE_REQUIRED_LIBRARIES  LIBRARIES  ${CMAKE_REQUIRED_LIBRARIES} CONFIGURATION "${CMAKE_TRY_COMPILE_CONFIGURATION}")
      set(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${_ADJUSTED_CMAKE_REQUIRED_LIBRARIES}")
    else(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES)
    endif(CMAKE_REQUIRED_LIBRARIES)
    try_compile(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/CheckVariableExists.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_VARIABLE_DEFINITIONS}
      "${CHECK_VARIABLE_EXISTS_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have variable ${VAR}")
      message(STATUS "Looking for ${VAR} - found")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the variable ${VAR} exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    else(${VARIABLE})
      set(${VARIABLE} "" CACHE INTERNAL "Have variable ${VAR}")
      message(STATUS "Looking for ${VAR} - not found")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the variable ${VAR} exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    endif(${VARIABLE})
  endif("${VARIABLE}" MATCHES "^${VARIABLE}$")
endmacro(CHECK_VARIABLE_EXISTS)
