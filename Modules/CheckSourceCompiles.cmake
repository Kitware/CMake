# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


#[=======================================================================[.rst:
CheckSourceCompiles
----------------------

.. versionadded:: 3.19

Check once if source code can be built for a given language.

.. command:: check_source_compiles

  .. code-block:: cmake

    check_source_compiles(<lang> <code> <resultVar>
                          [FAIL_REGEX <regex1> [<regex2>...]]
                          [SRC_EXT <extension>])

  Check once that the source supplied in ``<code>`` can be built for code
  language ``<lang>``. The result is stored in the internal cache variable
  specified by ``<resultVar>``, with boolean ``true`` for success and
  boolean ``false`` for failure.

  If ``FAIL_REGEX`` is provided, then failure is determined by checking
  if anything in the compiler output matches any of the specified regular
  expressions.

  By default, the test source file will be given a file extension that matches
  the requested language. The ``SRC_EXT`` option can be used to override this
  with ``.<extension>`` instead.

  The C example checks if the compiler supports the ``noreturn`` attribute:

  .. code-block:: cmake

    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

    check_source_compiles(C
    "#if !__has_c_attribute(noreturn)
    #error \"No noreturn attribute\"
    #endif"
    HAVE_NORETURN)

  The Fortran example checks if the compiler supports the ``pure`` procedure
  attribute:

  .. code-block:: cmake

    set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

    check_source_compiles(Fortran
    "pure subroutine foo()
    end subroutine"
    HAVE_PURE)

  Internally, :command:`try_compile` is used to compile the source. If
  :variable:`CMAKE_TRY_COMPILE_TARGET_TYPE` is set to ``EXECUTABLE`` (default),
  the source is compiled and linked as an executable program. If set to
  ``STATIC_LIBRARY``, the source is compiled but not linked. In any case, all
  functions must be declared as usual.

  See also :command:`check_source_runs` to run compiled source.

  The compile and link commands can be influenced by setting any of the
  following variables prior to calling ``check_source_compiles()``:

.. include:: /module/include/CMAKE_REQUIRED_FLAGS.rst

.. include:: /module/include/CMAKE_REQUIRED_DEFINITIONS.rst

.. include:: /module/include/CMAKE_REQUIRED_INCLUDES.rst

.. include:: /module/include/CMAKE_REQUIRED_LINK_OPTIONS.rst

.. include:: /module/include/CMAKE_REQUIRED_LIBRARIES.rst

.. include:: /module/include/CMAKE_REQUIRED_LINK_DIRECTORIES.rst

.. include:: /module/include/CMAKE_REQUIRED_QUIET.rst

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckSourceCompiles)

function(CHECK_SOURCE_COMPILES _lang _source _var)
  cmake_check_source_compiles(${_lang} "${_source}" ${_var} ${ARGN})
endfunction()
