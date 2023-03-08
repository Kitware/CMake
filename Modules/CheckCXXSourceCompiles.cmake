# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckCXXSourceCompiles
----------------------

Check if given C++ source compiles and links into an executable.

.. command:: check_cxx_source_compiles

  .. code-block:: cmake

    check_cxx_source_compiles(<code> <resultVar>
                              [FAIL_REGEX <regex1> [<regex2>...]])

  Check that the source supplied in ``<code>`` can be compiled as a C++ source
  file and linked as an executable (so it must contain at least a ``main()``
  function). The result will be stored in the internal cache variable specified
  by ``<resultVar>``, with a boolean true value for success and boolean false
  for failure. If ``FAIL_REGEX`` is provided, then failure is determined by
  checking if anything in the output matches any of the specified regular
  expressions.

  The check is only performed once, with the result cached in the variable named
  by ``<resultVar>``. Every subsequent CMake run will re-use this cached value
  rather than performing the check again, even if the ``<code>`` changes. In
  order to force the check to be re-evaluated, the variable named by
  ``<resultVar>`` must be manually removed from the cache.

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_cxx_source_compiles()``:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckSourceCompiles)

macro(CHECK_CXX_SOURCE_COMPILES SOURCE VAR)
  cmake_check_source_compiles(CXX "${SOURCE}" ${VAR} ${ARGN})
endmacro()
