# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindWget
--------

Find wget

This module looks for wget.  This module defines the following values:

::

  WGET_EXECUTABLE: the full path to the wget tool.
  WGET_FOUND: True if wget has been found.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/FindCygwin.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindMsys.cmake)

find_program(WGET_EXECUTABLE
  wget
  ${CYGWIN_INSTALL_PATH}/bin
  ${MSYS_INSTALL_PATH}/usr/bin
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wget DEFAULT_MSG WGET_EXECUTABLE)

mark_as_advanced( WGET_EXECUTABLE )

# WGET option is deprecated.
# use WGET_EXECUTABLE instead.
set (WGET ${WGET_EXECUTABLE})
