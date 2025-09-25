# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CMakeFindDependencyMacro
------------------------

.. command:: find_dependency

  The ``find_dependency()`` macro wraps a :command:`find_package` call for
  a package dependency:

  .. code-block:: cmake

    find_dependency(<dep> [...])

  It is designed to be used in a
  :ref:`Package Configuration File <Config File Packages>`
  (``<PackageName>Config.cmake``).  ``find_dependency`` forwards the correct
  parameters for ``QUIET`` and ``REQUIRED`` which were passed to
  the original :command:`find_package` call.  Any additional arguments
  specified are forwarded to :command:`find_package`.

  If the dependency could not be found it sets an informative diagnostic
  message and calls :command:`return` to end processing of the calling
  package configuration file and return to the :command:`find_package`
  command that loaded it.

  .. note::

    The call to :command:`return` makes this macro unsuitable to call
    from :ref:`Find Modules`.

Package Dependency Search Optimizations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If ``find_dependency`` is called with arguments identical to a previous
call in the same directory, perhaps due to diamond-shaped package
dependencies, the underlying call to :command:`find_package` is optimized
out.  This optimization is important to support large package dependency
graphs while avoiding a combinatorial explosion of repeated searches.
However, the heuristic cannot account for ambient variables that
affect package behavior, such as ``<PackageName>_USE_STATIC_LIBS``,
offered by some packages.  Therefore package configuration files should
avoid setting such variables before their calls to ``find_dependency``.

.. versionchanged:: 3.15
  Previously, the underlying call to :command:`find_package` was always
  optimized out if the package had already been found.  CMake 3.15
  removed the optimization to support cases in which ``find_dependency``
  call arguments request different components.

.. versionchanged:: 3.26
  The pre-3.15 optimization was restored, but with the above-described
  heuristic to account for varying ``find_dependency`` call arguments.

#]=======================================================================]

macro(find_dependency dep)
  string(SHA256 cmake_fd_call_hash "${dep};${ARGN};${${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED}")
  if(_CMAKE_${dep}_${cmake_fd_call_hash}_FOUND)
    unset(cmake_fd_call_hash)
  else()
    list(APPEND _CMAKE_${dep}_HASH_STACK ${cmake_fd_call_hash})
    set(cmake_fd_quiet_arg)
    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
      set(cmake_fd_quiet_arg QUIET)
    endif()
    set(cmake_fd_required_arg)
    if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
      set(cmake_fd_required_arg REQUIRED)
    endif()

    get_property(cmake_fd_alreadyTransitive GLOBAL PROPERTY
      _CMAKE_${dep}_TRANSITIVE_DEPENDENCY
      )

    find_package(${dep} ${ARGN}
      ${cmake_fd_quiet_arg}
      ${cmake_fd_required_arg}
      )
    list(POP_BACK _CMAKE_${dep}_HASH_STACK cmake_fd_call_hash)
    set("_CMAKE_${dep}_${cmake_fd_call_hash}_FOUND" "${${dep}_FOUND}")

    if(NOT DEFINED cmake_fd_alreadyTransitive OR cmake_fd_alreadyTransitive)
      set_property(GLOBAL PROPERTY _CMAKE_${dep}_TRANSITIVE_DEPENDENCY TRUE)
    endif()

    unset(cmake_fd_alreadyTransitive)
    unset(cmake_fd_call_hash)
    unset(cmake_fd_quiet_arg)
    unset(cmake_fd_required_arg)
    if (NOT ${dep}_FOUND)
      set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "${CMAKE_FIND_PACKAGE_NAME} could not be found because dependency ${dep} could not be found.")
      set(${CMAKE_FIND_PACKAGE_NAME}_FOUND False)
      return()
    endif()
  endif()
endmacro()
