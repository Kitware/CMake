# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFortranSourceRuns
----------------------

.. versionadded:: 3.14

Check if given Fortran source compiles and links into an executable and can
subsequently be run.

.. command:: check_fortran_source_runs

  .. code-block:: cmake

    check_fortran_source_runs(<code> <resultVar>
        [SRC_EXT <extension>])

  Check that the source supplied in ``<code>`` can be compiled as a Fortran source
  file, linked as an executable and then run. The ``<code>`` must be a Fortran
  ``program``.

  .. code-block:: cmake

    check_fortran_source_runs("program test
    real :: x[*]
    call co_sum(x)
    end program"
    HAVE_COARRAY)

  This command can help avoid costly build processes when a compiler lacks support
  for a necessary feature, or a particular vendor library is not compatible with
  the Fortran compiler version being used. Some of these failures only occur at runtime
  instead of linktime, and a trivial runtime example can catch the issue before the
  main build process.

  If the ``<code>`` could be built and run
  successfully, the internal cache variable specified by ``<resultVar>`` will
  be set to 1, otherwise it will be set to an value that evaluates to boolean
  false (e.g. an empty string or an error message).

  By default, the test source file will be given a ``.F90`` file extension. The
  ``SRC_EXT`` option can be used to override this with ``.<extension>`` instead.

  The check is only performed once, with the result cached in the variable named
  by ``<resultVar>``. Every subsequent CMake run will reuse this cached value
  rather than performing the check again, even if the ``<code>`` changes. In
  order to force the check to be re-evaluated, the variable named by
  ``<resultVar>`` must be manually removed from the cache.

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_fortran_source_runs()``:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckSourceRuns)

macro(CHECK_Fortran_SOURCE_RUNS SOURCE VAR)
  # Pass the SRC_EXT we used by default historically.
  # A user-provided SRC_EXT argument in ARGN will override ours.
  cmake_check_source_runs(Fortran "${SOURCE}" ${VAR} SRC_EXT "F90" ${ARGN})
endmacro()
