# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


#[=======================================================================[.rst:
CheckSourceCompiles
----------------------

.. versionadded:: 3.19

Check if given source compiles and links into an executable.

.. command:: check_source_compiles

  .. code-block:: cmake

    check_source_compiles(<lang> <code> <resultVar>
                          [FAIL_REGEX <regex1> [<regex2>...]]
                          [SRC_EXT <extension>])

  Check that the source supplied in ``<code>`` can be compiled as a source
  file for the requested language and linked as an executable. The result
  will be stored in the internal cache variable specified by ``<resultVar>``,
  with a boolean true value for success and boolean false for failure. If
  ``FAIL_REGEX`` is provided, then failure is determined by checking if
  anything in the compiler output matches any of the specified regular
  expressions.

  By default, the test source file will be given a file extension that matches
  the requested language. The ``SRC_EXT`` option can be used to override this
  with ``.<extension>`` instead.

  The ``<code>`` must contain a valid main program. For example:

  .. code-block:: cmake

    check_source_compiles(C
    "#include <stdlib.h>
    #include <stdnoreturn.h>
    noreturn void f(){ exit(0); }
    int main(void) { f(); return 1; }"
    HAVE_NORETURN)

    check_source_compiles(Fortran
    "program test
    error stop
    end program"
    HAVE_ERROR_STOP)

  The check is only performed once, with the result cached in the variable
  named by ``<resultVar>``. Every subsequent CMake run will re-use this cached
  value rather than performing the check again, even if the ``<code>`` changes.
  In order to force the check to be re-evaluated, the variable named by
  ``<resultVar>`` must be manually removed from the cache.

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_source_compiles()``:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckSourceCompiles)

function(CHECK_SOURCE_COMPILES _lang _source _var)
  cmake_check_source_compiles(${_lang} "${_source}" ${_var} ${ARGN})
endfunction()
