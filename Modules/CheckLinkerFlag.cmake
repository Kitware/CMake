# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckLinkerFlag
---------------

.. versionadded:: 3.18

Check whether the compiler supports a given link flag.

.. command:: check_linker_flag

  .. code-block:: cmake

    check_linker_flag(<lang> <flag> <var>)

Check that the link ``<flag>`` is accepted by the ``<lang>`` compiler without
a diagnostic.  Stores the result in an internal cache entry named ``<var>``.

This command temporarily sets the ``CMAKE_REQUIRED_LINK_OPTIONS`` variable
and calls the :command:`check_source_compiles` command from the
:module:`CheckSourceCompiles` module.  See that module's documentation
for a listing of variables that can otherwise modify the build.

The underlying implementation relies on the :prop_tgt:`LINK_OPTIONS` property
to check the specified flag. The ``LINKER:`` prefix, as described in the
:command:`target_link_options` command, can be used as well.

A positive result from this check indicates only that the compiler did not
issue a diagnostic message when given the link flag.  Whether the flag has any
effect or even a specific one is beyond the scope of this module.

.. note::
  Since the :command:`try_compile` command forwards flags from variables
  like :variable:`CMAKE_<LANG>_FLAGS`, unknown flags in such variables may
  cause a false negative for this check.
#]=======================================================================]

include_guard(GLOBAL)
include(Internal/CheckLinkerFlag)

function(CHECK_LINKER_FLAG _lang _flag _var)
  cmake_check_linker_flag(${_lang} "${_flag}" ${_var})
endfunction()
