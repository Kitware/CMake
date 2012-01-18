# - Find perl
# this module looks for Perl
#
#  PERL_EXECUTABLE     - the full path to perl
#  PERL_FOUND          - If false, don't attempt to use perl.
#  PERL_VERSION_STRING - version of perl found (since CMake 2.8.8)

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

INCLUDE(FindCygwin)

SET(PERL_POSSIBLE_BIN_PATHS
  ${CYGWIN_INSTALL_PATH}/bin
  )

IF(WIN32)
  GET_FILENAME_COMPONENT(
    ActivePerl_CurrentVersion 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl;CurrentVersion]" 
    NAME)
  SET(PERL_POSSIBLE_BIN_PATHS ${PERL_POSSIBLE_BIN_PATHS}
    "C:/Perl/bin" 
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\${ActivePerl_CurrentVersion}]/bin
    )
ENDIF(WIN32)

FIND_PROGRAM(PERL_EXECUTABLE
  NAMES perl
  PATHS ${PERL_POSSIBLE_BIN_PATHS}
  )

IF(PERL_EXECUTABLE)
  ### PERL_VERSION
  EXECUTE_PROCESS(
    COMMAND
      ${PERL_EXECUTABLE} -V:version
      OUTPUT_VARIABLE
        PERL_VERSION_OUTPUT_VARIABLE
      RESULT_VARIABLE
        PERL_VERSION_RESULT_VARIABLE
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  IF(NOT PERL_VERSION_RESULT_VARIABLE AND NOT PERL_VERSION_OUTPUT_VARIABLE MATCHES "^version='UNKNOWN'")
    STRING(REGEX REPLACE "version='([^']+)'.*" "\\1" PERL_VERSION_STRING ${PERL_VERSION_OUTPUT_VARIABLE})
  ELSE()
    EXECUTE_PROCESS(
      COMMAND ${PERL_EXECUTABLE} -v
      OUTPUT_VARIABLE PERL_VERSION_OUTPUT_VARIABLE
      RESULT_VARIABLE PERL_VERSION_RESULT_VARIABLE
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    IF(NOT PERL_VERSION_RESULT_VARIABLE AND PERL_VERSION_OUTPUT_VARIABLE MATCHES "This is perl.*[ \\(]v([0-9\\._]+)[ \\)]")
      STRING(REGEX REPLACE ".*This is perl.*[ \\(]v([0-9\\._]+)[ \\)].*" "\\1" PERL_VERSION_STRING ${PERL_VERSION_OUTPUT_VARIABLE})
    ELSEIF(NOT PERL_VERSION_RESULT_VARIABLE AND PERL_VERSION_OUTPUT_VARIABLE MATCHES "This is perl, version ([0-9\\._]+) +")
      STRING(REGEX REPLACE ".*This is perl, version ([0-9\\._]+) +.*" "\\1" PERL_VERSION_STRING ${PERL_VERSION_OUTPUT_VARIABLE})
    ENDIF()
  ENDIF()
ENDIF(PERL_EXECUTABLE)

# Deprecated settings for compatibility with CMake1.4
SET(PERL ${PERL_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set PERL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Perl
                                  REQUIRED_VARS PERL_EXECUTABLE
                                  VERSION_VAR PERL_VERSION_STRING)

MARK_AS_ADVANCED(PERL_EXECUTABLE)
