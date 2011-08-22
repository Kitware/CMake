# - Check if a symbol exists as a function, variable, or macro
# CHECK_SYMBOL_EXISTS(<symbol> <files> <variable>)
#
# Check that the <symbol> is available after including given header
# <files> and store the result in a <variable>.  Specify the list
# of files in one argument as a semicolon-separated list.
#
# If the header files define the symbol as a macro it is considered
# available and assumed to work.  If the header files declare the
# symbol as a function or variable then the symbol must also be
# available for linking.  If the symbol is a type or enum value
# it will not be recognized (consider using CheckTypeSize or
# CheckCSourceCompiles).
# If the check needs to be done in C++, consider using CHECK_CXX_SYMBOL_EXISTS(),
# which does the same as CHECK_SYMBOL_EXISTS(), but in C++.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

#=============================================================================
# Copyright 2003-2011 Kitware, Inc.
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

MACRO(CHECK_SYMBOL_EXISTS SYMBOL FILES VARIABLE)
  _CHECK_SYMBOL_EXISTS("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckSymbolExists.c" "${SYMBOL}" "${FILES}" "${VARIABLE}" )
ENDMACRO(CHECK_SYMBOL_EXISTS)

MACRO(_CHECK_SYMBOL_EXISTS SOURCEFILE SYMBOL FILES VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
    SET(MACRO_CHECK_SYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_SYMBOL_EXISTS_LIBS
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ELSE(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_SYMBOL_EXISTS_LIBS)
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CMAKE_SYMBOL_EXISTS_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CMAKE_SYMBOL_EXISTS_INCLUDES)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    FOREACH(FILE ${FILES})
      SET(CMAKE_CONFIGURABLE_FILE_CONTENT
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n")
    ENDFOREACH(FILE)
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT
      "${CMAKE_CONFIGURABLE_FILE_CONTENT}\nvoid cmakeRequireSymbol(int dummy,...){(void)dummy;}\nint main()\n{\n#ifndef ${SYMBOL}\n  cmakeRequireSymbol(0,&${SYMBOL});\n#endif\n  return 0;\n}\n")

    CONFIGURE_FILE("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
      "${SOURCEFILE}" @ONLY IMMEDIATE)

    MESSAGE(STATUS "Looking for ${SYMBOL}")
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      "${SOURCEFILE}"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_SYMBOL_EXISTS_FLAGS}
      "${CHECK_SYMBOL_EXISTS_LIBS}"
      "${CMAKE_SYMBOL_EXISTS_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have symbol ${SYMBOL}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the ${SYMBOL} "
        "exist passed with the following output:\n"
        "${OUTPUT}\nFile ${SOURCEFILE}:\n"
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${SYMBOL} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the ${SYMBOL} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nFile ${SOURCEFILE}:\n"
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(_CHECK_SYMBOL_EXISTS)
