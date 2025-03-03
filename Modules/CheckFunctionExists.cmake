# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFunctionExists
-------------------

Check once if a C function can be linked from system libraries.

.. command:: check_function_exists

  .. code-block:: cmake

    check_function_exists(<function> <variable>)

  Checks that the ``<function>`` is provided by libraries on the system and store
  the result in internal cache variable ``<variable>``.

The following variables may be set before calling this macro to modify the
way the check is run:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

.. note::

  Prefer using :module:`CheckSymbolExists` or :module:`CheckSourceCompiles`
  instead of this module, for the following reasons:

  * ``check_function_exists()`` can't detect functions that are inlined
    in headers or specified as a macro.

  * ``check_function_exists()`` can't detect anything in the 32-bit
    versions of the Win32 API, because of a mismatch in calling conventions.

  * ``check_function_exists()`` only verifies linking, it does not verify
    that the function is declared in system headers.
#]=======================================================================]

include_guard(GLOBAL)

macro(CHECK_FUNCTION_EXISTS FUNCTION VARIABLE)
  if(NOT DEFINED "${VARIABLE}" OR "x${${VARIABLE}}" STREQUAL "x${VARIABLE}")
    set(MACRO_CHECK_FUNCTION_DEFINITIONS
      "-DCHECK_FUNCTION_EXISTS=${FUNCTION} ${CMAKE_REQUIRED_FLAGS}")
    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Looking for ${FUNCTION}")
    endif()
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
      set(_CFE_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CFE_LINK_DIRECTORIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_FUNCTION_EXISTS_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_FUNCTION_EXISTS_ADD_INCLUDES)
    endif()

    if(CMAKE_C_COMPILER_LOADED)
      set(_cfe_source CheckFunctionExists.c)
    elseif(CMAKE_CXX_COMPILER_LOADED)
      set(_cfe_source CheckFunctionExists.cxx)
    else()
      message(FATAL_ERROR "CHECK_FUNCTION_EXISTS needs either C or CXX language enabled")
    endif()

    try_compile(${VARIABLE}
      SOURCE_FROM_FILE "${_cfe_source}" "${CMAKE_ROOT}/Modules/CheckFunctionExists.c"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_FUNCTION_EXISTS_ADD_LINK_OPTIONS}
      ${CHECK_FUNCTION_EXISTS_ADD_LIBRARIES}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      "${CHECK_FUNCTION_EXISTS_ADD_INCLUDES}"
      "${_CFE_LINK_DIRECTORIES}"
      )
    unset(_cfe_source)
    unset(_CFE_LINK_DIRECTORIES)

    if(${VARIABLE})
      set(${VARIABLE} 1 CACHE INTERNAL "Have function ${FUNCTION}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "found")
      endif()
    else()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "not found")
      endif()
      set(${VARIABLE} "" CACHE INTERNAL "Have function ${FUNCTION}")
    endif()
  endif()
endmacro()
