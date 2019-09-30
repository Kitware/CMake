# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckOBJCXXCompilerFlag
-----------------------

Check whether the Objective-C++ compiler supports a given flag.

.. command:: check_objcxx_compiler_flag

  .. code-block:: cmake

    check_objcxx_compiler_flag(<flag> <var>)

  Check that the ``<flag>`` is accepted by the compiler without
  a diagnostic.  Stores the result in an internal cache entry
  named ``<var>``.

This command temporarily sets the ``CMAKE_REQUIRED_DEFINITIONS`` variable
and calls the ``check_objcxx_source_compiles`` macro from the
:module:`CheckOBJCXXSourceCompiles` module.  See documentation of that
module for a listing of variables that can otherwise modify the build.

A positive result from this check indicates only that the compiler did not
issue a diagnostic message when given the flag.  Whether the flag has any
effect or even a specific one is beyond the scope of this module.

.. note::
  Since the :command:`try_compile` command forwards flags from variables
  like :variable:`CMAKE_OBJCXX_FLAGS <CMAKE_<LANG>_FLAGS>`, unknown flags
  in such variables may cause a false negative for this check.
#]=======================================================================]

include_guard(GLOBAL)
include(CheckOBJCXXSourceCompiles)
include(CMakeCheckCompilerFlagCommonPatterns)

macro (CHECK_OBJCXX_COMPILER_FLAG _FLAG _RESULT)
  set(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
  set(CMAKE_REQUIRED_DEFINITIONS "${_FLAG}")

  # Normalize locale during test compilation.
  set(_CheckOBJCXXCompilerFlag_LOCALE_VARS LC_ALL LC_MESSAGES LANG)
  foreach(v ${_CheckOBJCXXCompilerFlag_LOCALE_VARS})
    set(_CheckOBJCXXCompilerFlag_SAVED_${v} "$ENV{${v}}")
    set(ENV{${v}} OBJCXX)
  endforeach()
  CHECK_COMPILER_FLAG_COMMON_PATTERNS(_CheckOBJCXXCompilerFlag_COMMON_PATTERNS)
  CHECK_OBJCXX_SOURCE_COMPILES("#ifndef __OBJC__\n#  error \"Not an Objective-C++ compiler\"\n#endif\nint main(void) { return 0; }" ${_RESULT}
    # Some compilers do not fail with a bad flag
    FAIL_REGEX "command line option .* is valid for .* but not for Objective-C\\\\+\\\\+" # GNU
    FAIL_REGEX "argument unused during compilation: .*" # Clang
    ${_CheckOBJCXXCompilerFlag_COMMON_PATTERNS}
    )
  foreach(v ${_CheckOBJCXXCompilerFlag_LOCALE_VARS})
    set(ENV{${v}} ${_CheckOBJCXXCompilerFlag_SAVED_${v}})
    unset(_CheckOBJCXXCompilerFlag_SAVED_${v})
  endforeach()
  unset(_CheckOBJCXXCompilerFlag_LOCALE_VARS)
  unset(_CheckOBJCXXCompilerFlag_COMMON_PATTERNS)

  set (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
endmacro ()
