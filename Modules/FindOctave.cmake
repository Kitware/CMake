# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindOctave
----------

Finds GNU Octave interpreter, libraries and compilers.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``Octave::Interpreter``
  Octave interpreter (the main program)
``Octave::Octave``
  include directories and libraries

If no ``COMPONENTS`` are specified, ``Interpreter`` is assumed.

Result Variables
^^^^^^^^^^^^^^^^

``Octave_FOUND``
  Octave interpreter and/or libraries were found
``Octave_<component>_FOUND``
  Octave <component> specified was found

``Octave_EXECUTABLE``
  Octave interpreter
``Octave_INCLUDE_DIRS``
  include path for mex.h
``Octave_LIBRARIES``
  octinterp, octave libraries


Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Octave_INTERP_LIBRARY``
  path to the library octinterp
``Octave_OCTAVE_LIBRARY``
  path to the liboctave library

#]=======================================================================]

cmake_policy(VERSION 3.3)

unset(Octave_REQUIRED_VARS)
unset(Octave_Development_FOUND)
unset(Octave_Interpreter_FOUND)
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Interpreter)

if(Development IN_LIST Octave_FIND_COMPONENTS)
  find_program(Octave_CONFIG_EXECUTABLE
               NAMES octave-config)

  if(Octave_CONFIG_EXECUTABLE)

    execute_process(COMMAND ${Octave_CONFIG_EXECUTABLE} -p BINDIR
                    OUTPUT_VARIABLE Octave_BINARY_DIR
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${Octave_CONFIG_EXECUTABLE} -p OCTINCLUDEDIR
                    OUTPUT_VARIABLE Octave_INCLUDE_DIR
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND Octave_REQUIRED_VARS ${Octave_INCLUDE_DIR})

    execute_process(COMMAND ${Octave_CONFIG_EXECUTABLE} -p OCTLIBDIR
                    OUTPUT_VARIABLE Octave_LIB1
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${Octave_CONFIG_EXECUTABLE} -p LIBDIR
                    OUTPUT_VARIABLE Octave_LIB2
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    find_library(Octave_INTERP_LIBRARY
               NAMES octinterp
               PATHS ${Octave_LIB1} ${Octave_LIB2}
               NO_DEFAULT_PATH
              )
    find_library(Octave_OCTAVE_LIBRARY
                 NAMES octave
                 PATHS ${Octave_LIB1} ${Octave_LIB2}
                 NO_DEFAULT_PATH
                )
    list(APPEND Octave_REQUIRED_VARS ${Octave_OCTAVE_LIBRARY} ${Octave_INTERP_LIBRARY})

    if(Octave_REQUIRED_VARS)
      set(Octave_Development_FOUND true)
    endif()
  endif(Octave_CONFIG_EXECUTABLE)
endif()

if(Interpreter IN_LIST Octave_FIND_COMPONENTS)

  find_program(Octave_EXECUTABLE
               NAMES octave)

  list(APPEND Octave_REQUIRED_VARS ${Octave_EXECUTABLE})

endif()

if(Octave_EXECUTABLE)
  execute_process(COMMAND ${Octave_EXECUTABLE} -v
                  OUTPUT_VARIABLE Octave_VERSION
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)


  string(REGEX REPLACE "GNU Octave, version ([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" Octave_VERSION_MAJOR ${Octave_VERSION})
  string(REGEX REPLACE "GNU Octave, version [0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" Octave_VERSION_MINOR ${Octave_VERSION})
  string(REGEX REPLACE "GNU Octave, version [0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" Octave_VERSION_PATCH ${Octave_VERSION})

  set(Octave_VERSION ${Octave_VERSION_MAJOR}.${Octave_VERSION_MINOR}.${Octave_VERSION_PATCH})

  set(Octave_Interpreter_FOUND true)

endif(Octave_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Octave
  REQUIRED_VARS Octave_REQUIRED_VARS
  VERSION_VAR Octave_VERSION
  HANDLE_COMPONENTS)


if(Octave_Development_FOUND)
  set(Octave_LIBRARIES ${Octave_INTERP_LIBRARY} ${Octave_OCTAVE_LIBRARY})
  set(Octave_INCLUDE_DIRS ${Octave_INCLUDE_DIR})

  if(NOT TARGET Octave::Octave)
    add_library(Octave::Octave UNKNOWN IMPORTED)
    set_target_properties(Octave::Octave PROPERTIES
                          IMPORTED_LOCATION ${Octave_OCTAVE_LIBRARY}
                          INTERFACE_INCLUDE_DIRECTORIES ${Octave_INCLUDE_DIR}
                         )
  endif()

endif()


if(Octave_Interpreter_FOUND)
  if(NOT TARGET Octave::Interpreter)
    add_executable(Octave::Interpreter IMPORTED)
    set_target_properties(Octave::Interpreter PROPERTIES
                          IMPORTED_LOCATION ${Octave_EXECUTABLE}
                          VERSION ${Octave_VERSION})
  endif()
endif()

mark_as_advanced(
  Octave_CONFIG_EXECUTABLE
  Octave_INTERP_LIBRARY
  Octave_OCTAVE_LIBRARY
  Octave_INCLUDE_DIR
  Octave_VERSION_MAJOR
  Octave_VERSION_MINOR
  Octave_VERSION_PATCH
)
