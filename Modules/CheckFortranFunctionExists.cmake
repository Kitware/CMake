# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFortranFunctionExists
--------------------------

Check if a Fortran function exists.

.. command:: check_fortran_function_exists

  .. code-block:: cmake

    check_fortran_function_exists(<function> <result>)

  where

  ``<function>``
    the name of the Fortran function
  ``<result>``
    variable to store the result; will be created as an internal cache variable.

.. note::

  This command does not detect functions in Fortran modules. In general it is
  recommended to use :module:`CheckSourceCompiles` instead to determine if a
  Fortran function or subroutine is available.

The following variables may be set before calling this macro to modify
the way the check is run:

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt
#]=======================================================================]

include_guard(GLOBAL)

macro(CHECK_FORTRAN_FUNCTION_EXISTS FUNCTION VARIABLE)
  if(NOT DEFINED ${VARIABLE})
    message(CHECK_START "Looking for Fortran ${FUNCTION}")
    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_FUNCTION_EXISTS_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_FUNCTION_EXISTS_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_FUNCTION_EXISTS_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_FUNCTION_EXISTS_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_LINK_DIRECTORIES)
      set(_CFFE_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CFFE_LINK_DIRECTORIES)
    endif()
    set(__CheckFunction_testFortranCompilerSource
    "
      program TESTFortran
      external ${FUNCTION}
      call ${FUNCTION}()
      end program TESTFortran
    "
    )
    try_compile(${VARIABLE}
      SOURCE_FROM_VAR testFortranCompiler.f __CheckFunction_testFortranCompilerSource
      ${CHECK_FUNCTION_EXISTS_ADD_LINK_OPTIONS}
      ${CHECK_FUNCTION_EXISTS_ADD_LIBRARIES}
      CMAKE_FLAGS
      "${_CFFE_LINK_DIRECTORIES}"
    )
    unset(__CheckFunction_testFortranCompilerSource)
    unset(_CFFE_LINK_DIRECTORIES)
    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have Fortran function ${FUNCTION}")
      message(CHECK_PASS "found")
    else()
      message(CHECK_FAIL "not found")
      set(${VARIABLE} "" CACHE INTERNAL "Have Fortran function ${FUNCTION}")
    endif()
  endif()
endmacro()
