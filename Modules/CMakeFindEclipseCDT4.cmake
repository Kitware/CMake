
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

# This variable is used by the Eclipse generator and appended to the make invocation commands.
SET(CMAKE_ECLIPSE_MAKE_ARGUMENTS "" CACHE STRING "Additional command line arguments when Eclipse invokes make. Enter e.g. -j<some_number> to get parallel builds")

# This variable is used by the Eclipse generator in out-of-source builds only.
SET(ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT FALSE CACHE BOOL "If enabled, CMake will generate a source project for Eclipse in CMAKE_SOURCE_DIR")
MARK_AS_ADVANCED(ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT)

# The Eclipse generator needs to know the standard include path
# so that Eclipse ca find the headers at runtime and parsing etc. works better
# This is done here by actually running gcc with the options so it prints its
# system include directories, which are parsed then and stored in the cache.
MACRO(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang _resultIncludeDirs _resultDefines)
  SET(${_resultIncludeDirs})
  SET(_gccOutput)
  FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy" "\n" )

  IF (${_lang} STREQUAL "c++")
    SET(_compilerExecutable "${CMAKE_CXX_COMPILER}")
  ELSE (${_lang} STREQUAL "c++")
    SET(_compilerExecutable "${CMAKE_C_COMPILER}")
  ENDIF (${_lang} STREQUAL "c++")
  EXECUTE_PROCESS(COMMAND ${_compilerExecutable} -v -E -x ${_lang} -dD dummy
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles
                  ERROR_VARIABLE _gccOutput
                  OUTPUT_VARIABLE _gccStdout )
  FILE(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy")

  # First find the system include dirs:
  IF( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+ *\n) *End of (search) list" )

    # split the output into lines and then remove leading and trailing spaces from each of them:
    STRING(REGEX MATCHALL "[^\n]+\n" _includeLines "${CMAKE_MATCH_1}")
    FOREACH(nextLine ${_includeLines})
      STRING(STRIP "${nextLine}" _includePath)
      LIST(APPEND ${_resultIncludeDirs} "${_includePath}")
    ENDFOREACH(nextLine)

  ENDIF( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+ *\n) *End of (search) list" )


  # now find the builtin macros:
  STRING(REGEX MATCHALL "#define[^\n]+\n" _defineLines "${_gccStdout}")
# A few example lines which the regexp below has to match properly:
#  #define   MAX(a,b) ((a) > (b) ? (a) : (b))
#  #define __fastcall __attribute__((__fastcall__))
#  #define   FOO (23)
#  #define __UINTMAX_TYPE__ long long unsigned int
#  #define __UINTMAX_TYPE__ long long unsigned int
#  #define __i386__  1

  FOREACH(nextLine ${_defineLines})
    STRING(REGEX MATCH "^#define +([A-Za-z_][A-Za-z0-9_]*)(\\([^\\)]+\\))? +(.+) *$" _dummy "${nextLine}")
    SET(_name "${CMAKE_MATCH_1}${CMAKE_MATCH_2}")
    STRING(STRIP "${CMAKE_MATCH_3}" _value)
    #MESSAGE(STATUS "m1: -${CMAKE_MATCH_1}- m2: -${CMAKE_MATCH_2}- m3: -${CMAKE_MATCH_3}-")

    LIST(APPEND ${_resultDefines} "${_name}")
    IF(_value)
      LIST(APPEND ${_resultDefines} "${_value}")
    ELSE()
      LIST(APPEND ${_resultDefines} " ")
    ENDIF()
  ENDFOREACH(nextLine)

ENDMACRO(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang)

# Save the current LC_ALL, LC_MESSAGES, and LANG environment variables and set them
# to "C" that way GCC's "search starts here" text is in English and we can grok it.
SET(_orig_lc_all      $ENV{LC_ALL})
SET(_orig_lc_messages $ENV{LC_MESSAGES})
SET(_orig_lang        $ENV{LANG})
IF(_orig_lc_all)
  SET(ENV{LC_ALL}      C)
ENDIF(_orig_lc_all)
IF(_orig_lc_messages)
  SET(ENV{LC_MESSAGES} C)
ENDIF(_orig_lc_messages)
IF(_orig_lang)
  SET(ENV{LANG}        C)
ENDIF(_orig_lang)

# Now check for C, works for gcc and Intel compiler at least
IF (NOT CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS)
  IF ("${CMAKE_C_COMPILER_ID}" MATCHES GNU  OR  "${CMAKE_C_COMPILER_ID}" MATCHES Intel)
    _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c _dirs _defines)
    SET(CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS "${_dirs}" CACHE INTERNAL "C compiler system include directories")
    SET(CMAKE_ECLIPSE_C_SYSTEM_DEFINED_MACROS "${_defines}" CACHE INTERNAL "C compiler system defined macros")
  ENDIF ("${CMAKE_C_COMPILER_ID}" MATCHES GNU  OR  "${CMAKE_C_COMPILER_ID}" MATCHES Intel)
ENDIF (NOT CMAKE_ECLIPSE_C_SYSTEM_INCLUDE_DIRS)

# And now the same for C++
IF (NOT CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS)
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES GNU  OR  "${CMAKE_CXX_COMPILER_ID}" MATCHES Intel)
    _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c++ _dirs _defines)
    SET(CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS "${_dirs}" CACHE INTERNAL "CXX compiler system include directories")
    SET(CMAKE_ECLIPSE_CXX_SYSTEM_DEFINED_MACROS "${_defines}" CACHE INTERNAL "CXX compiler system defined macros")
  ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES GNU  OR  "${CMAKE_CXX_COMPILER_ID}" MATCHES Intel)
ENDIF (NOT CMAKE_ECLIPSE_CXX_SYSTEM_INCLUDE_DIRS)

# Restore original LC_ALL, LC_MESSAGES, and LANG
IF(_orig_lc_all)
  SET(ENV{LC_ALL}      ${_orig_lc_all})
ENDIF(_orig_lc_all)
IF(_orig_lc_messages)
  SET(ENV{LC_MESSAGES} ${_orig_lc_messages})
ENDIF(_orig_lc_messages)
IF(_orig_lang)
  SET(ENV{LANG}        ${_orig_lang})
ENDIF(_orig_lang)
