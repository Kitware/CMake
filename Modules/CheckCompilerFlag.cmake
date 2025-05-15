# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckCompilerFlag
---------------------

.. versionadded:: 3.19

Check once whether the ``<lang>`` compiler supports a given flag.

.. command:: check_compiler_flag

  .. code-block:: cmake

    check_compiler_flag(<lang> <flag> <resultVar>)

Check once that the ``<flag>`` is accepted by the ``<lang>`` compiler without
a diagnostic. The result is stored in the internal cache variable specified by
``<resultVar>``, with boolean ``true`` for success and boolean ``false`` for
failure.

``true`` indicates only that the compiler did not issue a diagnostic message
when given the flag. Whether the flag has any effect is beyond the scope of
this module.

Internally, :command:`try_compile` is used to perform the check. If
:variable:`CMAKE_TRY_COMPILE_TARGET_TYPE` is set to ``EXECUTABLE`` (default),
the check compiles and links an executable program. If set to
``STATIC_LIBRARY``, the check is compiled but not linked.

The compile and link commands can be influenced by setting any of the
following variables prior to calling ``check_compiler_flag()``. Unknown flags
in these variables can case a false negative result.

  .. include:: /module/include/CMAKE_REQUIRED_FLAGS.rst

  .. include:: /module/include/CMAKE_REQUIRED_DEFINITIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_INCLUDES.rst

  .. include:: /module/include/CMAKE_REQUIRED_LINK_OPTIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_LIBRARIES.rst

  .. include:: /module/include/CMAKE_REQUIRED_LINK_DIRECTORIES.rst

  .. include:: /module/include/CMAKE_REQUIRED_QUIET.rst

#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckCompilerFlag)

function(CHECK_COMPILER_FLAG _lang _flag _var)
  cmake_check_compiler_flag(${_lang} "${_flag}" ${_var})
endfunction()
