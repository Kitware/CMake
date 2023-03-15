# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFortranFunctionExists
--------------------------

Check if a Fortran function exists.

.. command:: CHECK_FORTRAN_FUNCTION_EXISTS

  .. code-block:: cmake

    CHECK_FORTRAN_FUNCTION_EXISTS(<function> <result>)

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

``CMAKE_REQUIRED_LINK_OPTIONS``
  .. versionadded:: 3.14
    A :ref:`;-list <CMake Language Lists>` of options to add to the link
    command (see :command:`try_compile` for further details).

``CMAKE_REQUIRED_LIBRARIES``
  A :ref:`;-list <CMake Language Lists>` of libraries to add to the link
  command. These can be the name of system libraries or they can be
  :ref:`Imported Targets <Imported Targets>` (see :command:`try_compile` for
  further details).
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
    )
    unset(__CheckFunction_testFortranCompilerSource)
    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have Fortran function ${FUNCTION}")
      message(CHECK_PASS "found")
    else()
      message(CHECK_FAIL "not found")
      set(${VARIABLE} "" CACHE INTERNAL "Have Fortran function ${FUNCTION}")
    endif()
  endif()
endmacro()
