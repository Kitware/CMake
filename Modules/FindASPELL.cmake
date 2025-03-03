# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindASPELL
----------

Try to find ASPELL

Once done this will define

::

  ASPELL_FOUND - system has ASPELL
  ASPELL_EXECUTABLE - the ASPELL executable
  ASPELL_INCLUDE_DIR - the ASPELL include directory
  ASPELL_LIBRARIES - The libraries needed to use ASPELL
  ASPELL_DEFINITIONS - Compiler switches required for using ASPELL
#]=======================================================================]

find_path(ASPELL_INCLUDE_DIR aspell.h )

find_program(ASPELL_EXECUTABLE
  NAMES aspell
)

find_library(ASPELL_LIBRARIES NAMES aspell aspell-15 libaspell-15 libaspell)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASPELL DEFAULT_MSG ASPELL_LIBRARIES ASPELL_INCLUDE_DIR ASPELL_EXECUTABLE)

mark_as_advanced(ASPELL_INCLUDE_DIR ASPELL_LIBRARIES ASPELL_EXECUTABLE)
