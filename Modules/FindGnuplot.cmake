# - this module looks for gnuplot
#
# Once done this will define
#
#  GNUPLOT_FOUND - system has Gnuplot
#  GNUPLOT_EXECUTABLE - the Gnuplot executable

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

include(FindCygwin)

find_program(GNUPLOT_EXECUTABLE
  NAMES
  gnuplot
  pgnuplot
  wgnupl32
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin
)

# for compatibility
set(GNUPLOT ${GNUPLOT_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set GNUPLOT_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gnuplot DEFAULT_MSG GNUPLOT_EXECUTABLE)

mark_as_advanced( GNUPLOT_EXECUTABLE )

