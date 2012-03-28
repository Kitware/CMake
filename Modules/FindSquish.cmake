#
# ---- Find Squish
# This module can be used to find Squish (currently support is aimed at version 3).
#
# ---- Variables and Macros
#  SQUISH_FOUND                    If false, don't try to use Squish
#
#  SQUISH_INSTALL_DIR              The Squish installation directory (containing bin, lib, etc)
#  SQUISH_SERVER_EXECUTABLE        The squishserver executable
#  SQUISH_CLIENT_EXECUTABLE        The squishrunner executable
#
#  SQUISH_INSTALL_DIR_FOUND        Was the install directory found?
#  SQUISH_SERVER_EXECUTABLE_FOUND  Was the server executable found?
#  SQUISH_CLIENT_EXECUTABLE_FOUND  Was the client executable found?
#
# macro SQUISH_ADD_TEST(testName applicationUnderTest testSuite testCase)
#
# ---- Typical Use
#  enable_testing()
#  find_package(Squish)
#  if (SQUISH_FOUND)
#    SQUISH_ADD_TEST(myTestName myApplication testSuiteName testCaseName)
#  endif ()
#

#=============================================================================
# Copyright 2008-2009 Kitware, Inc.
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

set(SQUISH_INSTALL_DIR_STRING "Directory containing the bin, doc, and lib directories for Squish; this should be the root of the installation directory.")
set(SQUISH_SERVER_EXECUTABLE_STRING "The squishserver executable program.")
set(SQUISH_CLIENT_EXECUTABLE_STRING "The squishclient executable program.")

# Search only if the location is not already known.
if(NOT SQUISH_INSTALL_DIR)
  # Get the system search path as a list.
  file(TO_CMAKE_PATH "$ENV{PATH}" SQUISH_INSTALL_DIR_SEARCH2)

  # Construct a set of paths relative to the system search path.
  set(SQUISH_INSTALL_DIR_SEARCH "")
  foreach(dir ${SQUISH_INSTALL_DIR_SEARCH2})
    set(SQUISH_INSTALL_DIR_SEARCH ${SQUISH_INSTALL_DIR_SEARCH} "${dir}/../lib/fltk")
  endforeach()
  string(REPLACE "//" "/" SQUISH_INSTALL_DIR_SEARCH "${SQUISH_INSTALL_DIR_SEARCH}")

  # Look for an installation
  find_path(SQUISH_INSTALL_DIR bin/squishrunner
    HINTS
    # Look for an environment variable SQUISH_INSTALL_DIR.
      ENV SQUISH_INSTALL_DIR

    # Look in places relative to the system executable search path.
    ${SQUISH_INSTALL_DIR_SEARCH}

    # Look in standard UNIX install locations.
    #/usr/local/squish

    DOC "The ${SQUISH_INSTALL_DIR_STRING}"
    )
endif()

# search for the executables
if(SQUISH_INSTALL_DIR)
  set(SQUISH_INSTALL_DIR_FOUND 1)

  # find the client program
  if(NOT SQUISH_CLIENT_EXECUTABLE)
    find_program(SQUISH_CLIENT_EXECUTABLE ${SQUISH_INSTALL_DIR}/bin/squishrunner DOC "The ${SQUISH_CLIENT_EXECUTABLE_STRING}")
  endif()

  # find the server program
  if(NOT SQUISH_SERVER_EXECUTABLE)
    find_program(SQUISH_SERVER_EXECUTABLE ${SQUISH_INSTALL_DIR}/bin/squishserver DOC "The ${SQUISH_SERVER_EXECUTABLE_STRING}")
  endif()

else()
  set(SQUISH_INSTALL_DIR_FOUND 0)
endif()

# record if executables are set
if(SQUISH_CLIENT_EXECUTABLE)
  set(SQUISH_CLIENT_EXECUTABLE_FOUND 1)
else()
  set(SQUISH_CLIENT_EXECUTABLE_FOUND 0)
endif()

if(SQUISH_SERVER_EXECUTABLE)
  set(SQUISH_SERVER_EXECUTABLE_FOUND 1)
else()
  set(SQUISH_SERVER_EXECUTABLE_FOUND 0)
endif()

# record if Squish was found
set(SQUISH_FOUND 1)
foreach(var SQUISH_INSTALL_DIR_FOUND SQUISH_CLIENT_EXECUTABLE_FOUND SQUISH_SERVER_EXECUTABLE_FOUND)
  if(NOT ${var})
    set(SQUISH_FOUND 0)
  endif()
endforeach()

macro(SQUISH_ADD_TEST testName testAUT testCase envVars testWraper)
  add_test(${testName}
    ${CMAKE_COMMAND} -V -VV
    "-Dsquish_aut:STRING=${testAUT}"
    "-Dsquish_server_executable:STRING=${SQUISH_SERVER_EXECUTABLE}"
    "-Dsquish_client_executable:STRING=${SQUISH_CLIENT_EXECUTABLE}"
    "-Dsquish_libqtdir:STRING=${QT_LIBRARY_DIR}"
    "-Dsquish_test_case:STRING=${testCase}"
    "-Dsquish_env_vars:STRING=${envVars}"
    "-Dsquish_wrapper:STRING=${testWraper}"
    -P "${CMAKE_ROOT}/Modules/SquishTestScript.cmake"
    )
  set_tests_properties(${testName}
    PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED;ERROR;FATAL"
    )
endmacro()

