# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckOBJCSourceRuns
-------------------

.. versionadded:: 3.16

Check once if given Objective-C source compiles and links into an executable and
can subsequently be run.

.. command:: check_objc_source_runs

  .. code-block:: cmake

    check_objc_source_runs(<code> <resultVar>)

  Check once that the source supplied in ``<code>`` can be built, linked as an
  executable, and then run. The ``<code>`` must contain at least a ``main()``
  function.

  The result is stored in the internal cache variable specified by
  ``<resultVar>``. Success of build and run is indicated by boolean ``true``.
  Failure to build or run is indicated by boolean ``false`` such as an empty
  string or an error message.

  See also :command:`check_source_runs` for a more general command syntax.

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_objc_source_runs()``

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckSourceRuns)

macro(CHECK_OBJC_SOURCE_RUNS SOURCE VAR)
  set(_CheckSourceRuns_old_signature 1)
  cmake_check_source_runs(OBJC "${SOURCE}" ${VAR} ${ARGN})
  unset(_CheckSourceRuns_old_signature)
endmacro()
