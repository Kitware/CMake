# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


#[=======================================================================[.rst:
CheckSourceRuns
-------------------

.. versionadded:: 3.19

Check if given source compiles and links into an executable and can
subsequently be run.

.. command:: check_source_runs

  .. code-block:: cmake

    check_source_runs(<lang> <code> <resultVar>
                      [SRC_EXT <extension>])

  Check once that the ``<lang>`` source supplied in ``<code>`` can be built,
  linked as an executable, and then run. The ``<code>`` must contain at least
  a ``main()`` function, or in Fortran a ``program``.

  The result is stored in the internal cache variable specified by
  ``<resultVar>``. If the code builds and runs with exit code ``0``, success is
  indicated by boolean ``true``. Failure to build or run is indicated by boolean
  ``false``, such as an empty string or an error message.

  By default, the test source file will be given a file extension that matches
  the requested language. The ``SRC_EXT`` option can be used to override this
  with ``.<extension>`` instead.

  The ``<code>`` must contain a valid main program. For example:

  .. code-block:: cmake

    check_source_runs(C
    "#include <stdlib.h>
    #include <stdnoreturn.h>
    noreturn void f(){ exit(0); }
    int main(void) { f(); return 1; }"
    HAVE_NORETURN)

    check_source_runs(Fortran
    "program test
    real :: x[*]
    call co_sum(x)
    end program"
    HAVE_COARRAY)

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_source_runs()``

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

function(CHECK_SOURCE_RUNS _lang _source _var)
  cmake_check_source_runs(${_lang} "${_source}" ${_var} ${ARGN})
endfunction()
