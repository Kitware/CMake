# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard(GLOBAL)

#[=======================================================================[.rst:
CMakePushCheckState
-------------------

This module provides macros for managing the state of variables that influence
how various CMake check commands (e.g., ``check_symbol_exists()``, etc.) are
performed.  These macros save, reset, and restore the following variables:

* ``CMAKE_REQUIRED_FLAGS``
* ``CMAKE_REQUIRED_DEFINITIONS``
* ``CMAKE_REQUIRED_INCLUDES``
* ``CMAKE_REQUIRED_LINK_OPTIONS``
* ``CMAKE_REQUIRED_LIBRARIES``
* ``CMAKE_REQUIRED_LINK_DIRECTORIES``
* ``CMAKE_REQUIRED_QUIET``
* ``CMAKE_EXTRA_INCLUDE_FILES``

Macros
^^^^^^

.. command:: cmake_push_check_state

  Saves (pushes) the current states of the above variables onto a stack.  This
  is typically used to preserve the current configuration before making
  temporary modifications for specific checks.

  .. code-block:: cmake

    cmake_push_check_state([RESET])

  ``RESET``
    When this option is specified, the macro not only saves the current states
    of the listed variables but also resets them to empty, allowing them to be
    reconfigured from a clean state.

.. command:: cmake_reset_check_state

  Resets (clears) the contents of the variables listed above to empty states.

  .. code-block:: cmake

    cmake_reset_check_state()

  This macro can be used, for example, when performing multiple sequential
  checks that require entirely new configurations, ensuring no previous
  configuration unintentionally carries over.

.. command:: cmake_pop_check_state

  Restores the states of the variables listed above to their values at the time
  of the most recent ``cmake_push_check_state()`` call.

  .. code-block:: cmake

    cmake_pop_check_state()

  This macro is used to revert temporary changes made during a check.  To
  prevent unexpected behavior, pair each ``cmake_push_check_state()`` with a
  corresponding ``cmake_pop_check_state()``.

These macros are useful for scoped configuration, for example, in
:ref:`Find modules <Find Modules>` or when performing checks in a controlled
environment, ensuring that temporary modifications are isolated to the scope of
the check and do not propagate into other parts of the build system.

.. note::

  Other CMake variables, such as ``CMAKE_<LANG>_FLAGS``, propagate to all checks
  regardless of these macros, as those fundamental variables are designed to
  influence the global state of the build system.

Examples
^^^^^^^^

.. code-block:: cmake

  include(CMakePushCheckState)

  # Save and reset the current state
  cmake_push_check_state(RESET)

  # Perform check with specific compile definitions
  set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  include(CheckSymbolExists)
  check_symbol_exists(memfd_create "sys/mman.h" HAVE_MEMFD_CREATE)

  # Restore the original state
  cmake_pop_check_state()

Variable states can be pushed onto the stack multiple times, allowing for nested
or sequential configurations.  Each ``cmake_pop_check_state()`` restores the
most recent pushed states.

.. code-block:: cmake

  include(CMakePushCheckState)

  # Save and reset the current state
  cmake_push_check_state(RESET)

  # Perform the first check with additional libraries
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
  include(CheckSymbolExists)
  check_symbol_exists(dlopen "dlfcn.h" HAVE_DLOPEN)

  # Save current state
  cmake_push_check_state()

  # Perform the second check with libraries and additional compile definitions
  set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  check_symbol_exists(dladdr "dlfcn.h" HAVE_DLADDR)

  message(STATUS "${CMAKE_REQUIRED_DEFINITIONS}")
  # Output: -D_GNU_SOURCE

  # Restore the previous state
  cmake_pop_check_state()

  message(STATUS "${CMAKE_REQUIRED_DEFINITIONS}")
  # Output here is empty

  # Reset variables to prepare for the next check
  cmake_reset_check_state()

  # Perform the next check only with additional compile definitions
  set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  check_symbol_exists(dl_iterate_phdr "link.h" HAVE_DL_ITERATE_PHDR)

  # Restore the original state
  cmake_pop_check_state()
#]=======================================================================]

macro(CMAKE_RESET_CHECK_STATE)

  set(CMAKE_EXTRA_INCLUDE_FILES)
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_DEFINITIONS)
  set(CMAKE_REQUIRED_LINK_OPTIONS)
  set(CMAKE_REQUIRED_LIBRARIES)
  set(CMAKE_REQUIRED_LINK_DIRECTORIES)
  set(CMAKE_REQUIRED_FLAGS)
  set(CMAKE_REQUIRED_QUIET)

endmacro()

macro(CMAKE_PUSH_CHECK_STATE)

  if(NOT DEFINED _CMAKE_PUSH_CHECK_STATE_COUNTER)
    set(_CMAKE_PUSH_CHECK_STATE_COUNTER 0)
  endif()

  math(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}+1")

  set(_CMAKE_EXTRA_INCLUDE_FILES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}        ${CMAKE_EXTRA_INCLUDE_FILES})
  set(_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}          ${CMAKE_REQUIRED_INCLUDES})
  set(_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}       ${CMAKE_REQUIRED_DEFINITIONS})
  set(_CMAKE_REQUIRED_LINK_OPTIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}      ${CMAKE_REQUIRED_LINK_OPTIONS})
  set(_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}         ${CMAKE_REQUIRED_LIBRARIES})
  set(_CMAKE_REQUIRED_LINK_DIRECTORIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}  ${CMAKE_REQUIRED_LINK_DIRECTORIES})
  set(_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}             ${CMAKE_REQUIRED_FLAGS})
  set(_CMAKE_REQUIRED_QUIET_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}             ${CMAKE_REQUIRED_QUIET})

  if (${ARGC} GREATER 0 AND "${ARGV0}" STREQUAL "RESET")
    cmake_reset_check_state()
  endif()

endmacro()

macro(CMAKE_POP_CHECK_STATE)

# don't pop more than we pushed
  if("${_CMAKE_PUSH_CHECK_STATE_COUNTER}" GREATER "0")

    set(CMAKE_EXTRA_INCLUDE_FILES       ${_CMAKE_EXTRA_INCLUDE_FILES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_INCLUDES         ${_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_DEFINITIONS      ${_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_LINK_OPTIONS     ${_CMAKE_REQUIRED_LINK_OPTIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_LIBRARIES        ${_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_LINK_DIRECTORIES ${_CMAKE_REQUIRED_LINK_DIRECTORIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_FLAGS            ${_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
    set(CMAKE_REQUIRED_QUIET            ${_CMAKE_REQUIRED_QUIET_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})

    math(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}-1")
  endif()

endmacro()
