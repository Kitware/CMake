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

include("${CMAKE_CURRENT_LIST_DIR}/CMakeExpandImportedTargets.cmake")


macro(CHECK_SYMBOL_EXISTS SYMBOL FILES VARIABLE)
  _CHECK_SYMBOL_EXISTS("${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckSymbolExists.c" "${SYMBOL}" "${FILES}" "${VARIABLE}" )
endmacro(CHECK_SYMBOL_EXISTS)

macro(_CHECK_SYMBOL_EXISTS SOURCEFILE SYMBOL FILES VARIABLE)
  if("${VARIABLE}" MATCHES "^${VARIABLE}$")
    set(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
    set(MACRO_CHECK_SYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    if(CMAKE_REQUIRED_LIBRARIES)
      # this one translates potentially used imported library targets to their files on disk
      CMAKE_EXPAND_IMPORTED_TARGETS(_ADJUSTED_CMAKE_REQUIRED_LIBRARIES  LIBRARIES  ${CMAKE_REQUIRED_LIBRARIES} CONFIGURATION "${CMAKE_TRY_COMPILE_CONFIGURATION}")
      set(CHECK_SYMBOL_EXISTS_LIBS
        "-DLINK_LIBRARIES:STRING=${_ADJUSTED_CMAKE_REQUIRED_LIBRARIES}")
    else(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_SYMBOL_EXISTS_LIBS)
    endif(CMAKE_REQUIRED_LIBRARIES)
    if(CMAKE_REQUIRED_INCLUDES)
      set(CMAKE_SYMBOL_EXISTS_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else(CMAKE_REQUIRED_INCLUDES)
      set(CMAKE_SYMBOL_EXISTS_INCLUDES)
    endif(CMAKE_REQUIRED_INCLUDES)
    foreach(FILE ${FILES})
      set(CMAKE_CONFIGURABLE_FILE_CONTENT
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n")
    endforeach(FILE)
    set(CMAKE_CONFIGURABLE_FILE_CONTENT
      "${CMAKE_CONFIGURABLE_FILE_CONTENT}\nint main(int argc, char** argv)\n{\n  (void)argv;\n#ifndef ${SYMBOL}\n  return ((int*)(&${SYMBOL}))[argc];\n#else\n  (void)argc;\n  return 0;\n#endif\n}\n")

    configure_file("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
      "${SOURCEFILE}" @ONLY IMMEDIATE)

    message(STATUS "Looking for ${SYMBOL}")
    try_compile(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      "${SOURCEFILE}"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_SYMBOL_EXISTS_FLAGS}
      "${CHECK_SYMBOL_EXISTS_LIBS}"
      "${CMAKE_SYMBOL_EXISTS_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)
    if(${VARIABLE})
      message(STATUS "Looking for ${SYMBOL} - found")
      set(${VARIABLE} 1 CACHE INTERNAL "Have symbol ${SYMBOL}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if the ${SYMBOL} "
        "exist passed with the following output:\n"
        "${OUTPUT}\nFile ${SOURCEFILE}:\n"
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    else(${VARIABLE})
      message(STATUS "Looking for ${SYMBOL} - not found.")
      set(${VARIABLE} "" CACHE INTERNAL "Have symbol ${SYMBOL}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if the ${SYMBOL} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nFile ${SOURCEFILE}:\n"
        "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    endif(${VARIABLE})
  endif("${VARIABLE}" MATCHES "^${VARIABLE}$")
endmacro(_CHECK_SYMBOL_EXISTS)
