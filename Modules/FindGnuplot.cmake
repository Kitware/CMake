# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGnuplot
-----------

This module looks for gnuplot.

Result Variables
^^^^^^^^^^^^^^^^

``GNUPLOT_FOUND``
  System has Gnuplot.

``GNUPLOT_EXECUTABLE``
  The Gnuplot executable.

``GNUPLOT_VERSION_STRING``
  The version of Gnuplot found.

  .. note:: Version string detection will not work for old versions like 3.7.1.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/FindCygwin.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindMsys.cmake)

find_program(GNUPLOT_EXECUTABLE
  NAMES
  gnuplot
  pgnuplot
  wgnupl32
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin
  ${MSYS_INSTALL_PATH}/usr/bin
)

if (GNUPLOT_EXECUTABLE)
    execute_process(COMMAND "${GNUPLOT_EXECUTABLE}" --version
                  OUTPUT_VARIABLE GNUPLOT_OUTPUT_VARIABLE
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX REPLACE "^gnuplot ([0-9\\.]+)( patchlevel )?" "\\1." GNUPLOT_VERSION_STRING "${GNUPLOT_OUTPUT_VARIABLE}")
    string(REGEX REPLACE "\\.$" "" GNUPLOT_VERSION_STRING "${GNUPLOT_VERSION_STRING}")
    unset(GNUPLOT_OUTPUT_VARIABLE)
endif()

# for compatibility
set(GNUPLOT ${GNUPLOT_EXECUTABLE})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gnuplot
                                  REQUIRED_VARS GNUPLOT_EXECUTABLE
                                  VERSION_VAR GNUPLOT_VERSION_STRING)

mark_as_advanced( GNUPLOT_EXECUTABLE )
