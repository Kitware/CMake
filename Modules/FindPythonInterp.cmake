# - Find python interpreter
# This module finds if Python interpreter is installed and determines where the
# executables are. This code sets the following variables:
#
#  PYTHONINTERP_FOUND         - Was the Python executable found
#  PYTHON_EXECUTABLE          - path to the Python interpreter
#  Python_ADDITIONAL_VERSIONS - list of additional Python versions to search for
#

#=============================================================================
# Copyright 2005-2010 Kitware, Inc.
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

# Set up the versions we know about, in the order we will search. Always add
# the user supplied additional versions to the front.
set(_Python_VERSIONS
  ${Python_ADDITIONAL_VERSIONS}
  2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)

# Run first with the Python version in the executable
foreach(_CURRENT_VERSION ${_Python_VERSIONS})
  set(_Python_NAMES python${_CURRENT_VERSION})
  if(WIN32)
    list(APPEND _Python_NAMES python)
  endif()
  find_program(PYTHON_EXECUTABLE
    NAMES ${_Python_NAMES}
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]
    )
endforeach()
# Now without any version if we still haven't found it
if(NOT PYTHON_EXECUTABLE)
  find_program(PYTHON_EXECUTABLE NAMES python)
endif()


# handle the QUIETLY and REQUIRED arguments and set PYTHONINTERP_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonInterp DEFAULT_MSG PYTHON_EXECUTABLE)

mark_as_advanced(PYTHON_EXECUTABLE)
