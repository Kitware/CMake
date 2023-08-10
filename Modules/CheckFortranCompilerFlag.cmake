# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFortranCompilerFlag
------------------------

.. versionadded:: 3.3

Check whether the Fortran compiler supports a given flag.

.. command:: check_fortran_compiler_flag

  .. code-block:: cmake

    check_fortran_compiler_flag(<flag> <resultVar>)

  Check that the ``<flag>`` is accepted by the compiler without
  a diagnostic.  Stores the result in an internal cache entry
  named ``<resultVar>``.

A positive result from this check indicates only that the compiler did not
issue a diagnostic message when given the flag.  Whether the flag has any
effect or even a specific one is beyond the scope of this module.

The check is only performed once, with the result cached in the variable named
by ``<resultVar>``. Every subsequent CMake run will re-use this cached value
rather than performing the check again, even if the ``<code>`` changes. In
order to force the check to be re-evaluated, the variable named by
``<resultVar>`` must be manually removed from the cache.

The compile and link commands can be influenced by setting any of the
following variables prior to calling ``check_fortran_compiler_flag()``

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckCompilerFlag)

macro (CHECK_FORTRAN_COMPILER_FLAG _FLAG _RESULT)
  cmake_check_compiler_flag(Fortran "${_FLAG}" ${_RESULT})
endmacro ()

# FIXME(#24994): The following module is included only for compatibility
# with projects that accidentally relied on it with CMake 3.26 and below.
include(CheckFortranSourceCompiles)
