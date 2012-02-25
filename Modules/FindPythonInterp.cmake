# - Find python interpreter
# This module finds if Python interpreter is installed and determines where the
# executables are. This code sets the following variables:
#
#  PYTHONINTERP_FOUND         - Was the Python executable found
#  PYTHON_EXECUTABLE          - path to the Python interpreter
#
#  PYTHON_VERSION_STRING      - Python version found e.g. 2.5.2
#  PYTHON_VERSION_MAJOR       - Python major version found e.g. 2
#  PYTHON_VERSION_MINOR       - Python minor version found e.g. 5
#  PYTHON_VERSION_PATCH       - Python patch version found e.g. 2
#
#  Python_ADDITIONAL_VERSIONS - list of additional Python versions to search for

#=============================================================================
# Copyright 2005-2010 Kitware, Inc.
# Copyright 2011 Bjoern Ricks <bjoern.ricks@gmail.com>
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

unset(_Python_NAMES)

set(_PYTHON1_VERSIONS 1.6 1.5)
set(_PYTHON2_VERSIONS 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0)
set(_PYTHON3_VERSIONS 3.3 3.2 3.1 3.0)

if(PythonInterp_FIND_VERSION)
    if(PythonInterp_FIND_VERSION MATCHES "^[0-9]+\\.[0-9]+(\\.[0-9]+.*)?$")
        string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" _PYTHON_FIND_MAJ_MIN "${PythonInterp_FIND_VERSION}")
        string(REGEX REPLACE "^([0-9]+).*" "\\1" _PYTHON_FIND_MAJ "${_PYTHON_FIND_MAJ_MIN}")
        list(APPEND _Python_NAMES python${_PYTHON_FIND_MAJ_MIN} python${_PYTHON_FIND_MAJ})
        unset(_PYTHON_FIND_OTHER_VERSIONS)
        if(NOT PythonInterp_FIND_VERSION_EXACT)
            foreach(_PYTHON_V ${_PYTHON${_PYTHON_FIND_MAJ}_VERSIONS})
                if(NOT _PYTHON_V VERSION_LESS _PYTHON_FIND_MAJ_MIN)
                    list(APPEND _PYTHON_FIND_OTHER_VERSIONS ${_PYTHON_V})
                endif()
             endforeach()
        endif(NOT PythonInterp_FIND_VERSION_EXACT)
        unset(_PYTHON_FIND_MAJ_MIN)
        unset(_PYTHON_FIND_MAJ)
    else(PythonInterp_FIND_VERSION MATCHES "^[0-9]+\\.[0-9]+(\\.[0-9]+.*)?$")
        list(APPEND _Python_NAMES python${PythonInterp_FIND_VERSION})
        set(_PYTHON_FIND_OTHER_VERSIONS ${_PYTHON${PythonInterp_FIND_VERSION}_VERSIONS})
    endif(PythonInterp_FIND_VERSION MATCHES "^[0-9]+\\.[0-9]+(\\.[0-9]+.*)?$")
else(PythonInterp_FIND_VERSION)
    set(_PYTHON_FIND_OTHER_VERSIONS ${_PYTHON3_VERSIONS} ${_PYTHON2_VERSIONS} ${_PYTHON1_VERSIONS})
endif(PythonInterp_FIND_VERSION)

list(APPEND _Python_NAMES python)

# Search for the current active python version first
find_program(PYTHON_EXECUTABLE NAMES ${_Python_NAMES})

# Set up the versions we know about, in the order we will search. Always add
# the user supplied additional versions to the front.
set(_Python_VERSIONS
  ${Python_ADDITIONAL_VERSIONS}
  ${_PYTHON_FIND_OTHER_VERSIONS}
  )

unset(_PYTHON_FIND_OTHER_VERSIONS)
unset(_PYTHON1_VERSIONS)
unset(_PYTHON2_VERSIONS)
unset(_PYTHON3_VERSIONS)

# Search for newest python version if python executable isn't found
if(NOT PYTHON_EXECUTABLE)
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
endif()

# determine python version string
if(PYTHON_EXECUTABLE)
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" --version
                    OUTPUT_VARIABLE _VERSION_O
                    ERROR_VARIABLE _VERSION
                    RESULT_VARIABLE _PYTHON_VERSION_RESULT
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_STRIP_TRAILING_WHITESPACE)
    if(_PYTHON_VERSION_RESULT)
message(STATUS "'python --version' process result is ${_PYTHON_VERSION_RESULT}, stdout is '${_VERSION_O}', stderr is '${_VERSION}'")
        execute_process(COMMAND "${PYTHON_EXECUTABLE}" -V
                       OUTPUT_VARIABLE _VERSION_O
                        ERROR_VARIABLE _VERSION
                        RESULT_VARIABLE _PYTHON_VERSION_RESULT
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_STRIP_TRAILING_WHITESPACE)
    endif(_PYTHON_VERSION_RESULT)
    if(NOT _PYTHON_VERSION_RESULT AND _VERSION MATCHES "^Python [0-9]+\\.[0-9]+.*")
        string(REPLACE "Python " "" PYTHON_VERSION_STRING "${_VERSION}")
        string(REGEX REPLACE "^([0-9]+)\\.[0-9]+.*" "\\1" PYTHON_VERSION_MAJOR "${PYTHON_VERSION_STRING}")
        string(REGEX REPLACE "^[0-9]+\\.([0-9])+.*" "\\1" PYTHON_VERSION_MINOR "${PYTHON_VERSION_STRING}")
        if(PYTHON_VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+.*")
            string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" PYTHON_VERSION_PATCH "${PYTHON_VERSION_STRING}")
        endif()
else()
message(STATUS "process result is ${_PYTHON_VERSION_RESULT}, stdout is '${_VERSION_O}', stderr is '${_VERSION}'")
    endif()
    unset(_PYTHON_VERSION_RESULT)
    unset(_VERSION)
endif(PYTHON_EXECUTABLE)

# handle the QUIETLY and REQUIRED arguments and set PYTHONINTERP_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonInterp REQUIRED_VARS PYTHON_EXECUTABLE VERSION_VAR PYTHON_VERSION_STRING)

mark_as_advanced(PYTHON_EXECUTABLE)
