# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


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

  Check that the source supplied in ``<code>`` can be compiled as a source
  file for the requested language, linked as an executable and then run.
  The ``<code>`` must contain at least a ``main()`` function. If the ``<code>``
  could be built and run successfully, the internal cache variable specified by
  ``<resultVar>`` will be set to 1, otherwise it will be set to an value that
  evaluates to boolean false (e.g. an empty string or an error message).

  By default, the test source file will be given a file extension that matches
  the requested language. The ``SRC_EXT`` option can be used to override this
  with ``.<extension>`` instead.

  The underlying check is performed by the :command:`try_run` command. The
  compile and link commands can be influenced by setting any of the following
  variables prior to calling ``check_objc_source_runs()``:

  ``CMAKE_REQUIRED_FLAGS``
    Additional flags to pass to the compiler. Note that the contents of
    :variable:`CMAKE_OBJC_FLAGS <CMAKE_<LANG>_FLAGS>` and its associated
    configuration-specific variable are automatically added to the compiler
    command before the contents of ``CMAKE_REQUIRED_FLAGS``.

  ``CMAKE_REQUIRED_DEFINITIONS``
    A :ref:`;-list <CMake Language Lists>` of compiler definitions of the form
    ``-DFOO`` or ``-DFOO=bar``. A definition for the name specified by
    ``<resultVar>`` will also be added automatically.

  ``CMAKE_REQUIRED_INCLUDES``
    A :ref:`;-list <CMake Language Lists>` of header search paths to pass to
    the compiler. These will be the only header search paths used by
    ``try_run()``, i.e. the contents of the :prop_dir:`INCLUDE_DIRECTORIES`
    directory property will be ignored.

  ``CMAKE_REQUIRED_LINK_OPTIONS``
    A :ref:`;-list <CMake Language Lists>` of options to add to the link
    command (see :command:`try_run` for further details).

  ``CMAKE_REQUIRED_LIBRARIES``
    A :ref:`;-list <CMake Language Lists>` of libraries to add to the link
    command. These can be the name of system libraries or they can be
    :ref:`Imported Targets <Imported Targets>` (see :command:`try_run` for
    further details).

  ``CMAKE_REQUIRED_QUIET``
    If this variable evaluates to a boolean true value, all status messages
    associated with the check will be suppressed.

  The check is only performed once, with the result cached in the variable
  named by ``<resultVar>``. Every subsequent CMake run will re-use this cached
  value rather than performing the check again, even if the ``<code>`` changes.
  In order to force the check to be re-evaluated, the variable named by
  ``<resultVar>`` must be manually removed from the cache.

#]=======================================================================]

include_guard(GLOBAL)

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW) # if() quoted variables not dereferenced
cmake_policy(SET CMP0057 NEW) # if() supports IN_LIST

function(CHECK_SOURCE_RUNS _lang _source _var)
  if(NOT DEFINED "${_var}")

    if(_lang STREQUAL C)
      set(_lang_textual "C")
      set(_lang_ext "c")
    elseif(_lang STREQUAL CXX)
      set(_lang_textual "C++")
      set(_lang_ext "cxx")
    elseif(_lang STREQUAL CUDA)
      set(_lang_textual "CUDA")
      set(_lang_ext "cu")
    elseif(_lang STREQUAL Fortran)
      set(_lang_textual "Fortran")
      set(_lang_ext "F")
    elseif(_lang STREQUAL OBJC)
      set(_lang_textual "Objective-C")
      set(_lang_ext "m")
    elseif(_lang STREQUAL OBJCXX)
      set(_lang_textual "Objective-C++")
      set(_lang_ext "mm")
    else()
      message (SEND_ERROR "check_source_runs: ${_lang}: unknown language.")
      return()
    endif()

    get_property (_supported_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    if (NOT _lang IN_LIST _supported_languages)
      message (SEND_ERROR "check_source_runs: ${_lang}: needs to be enabled before use.")
      return()
    endif()

    set(_FAIL_REGEX)
    set(_SRC_EXT)
    set(_key)
    foreach(arg ${ARGN})
      if("${arg}" MATCHES "^(SRC_EXT)$")
        set(_key "${arg}")
      elseif(_key)
        list(APPEND _${_key} "${arg}")
      else()
        message(FATAL_ERROR "Unknown argument:\n  ${arg}\n")
      endif()
    endforeach()

    if(NOT _SRC_EXT)
      set(_SRC_EXT ${_lang_ext})
    endif()

    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES)
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.${_SRC_EXT}"
      "${_source}\n")

    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Performing Test ${_var}")
    endif()
    try_run(${_var}_EXITCODE ${_var}_COMPILED
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.${_SRC_EXT}
      COMPILE_DEFINITIONS -D${_var} ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_${_lang}_SOURCE_COMPILES_ADD_LINK_OPTIONS}
      ${CHECK_${_lang}_SOURCE_COMPILES_ADD_LIBRARIES}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${CMAKE_REQUIRED_FLAGS}
      -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
      "${CHECK_${_lang}_SOURCE_COMPILES_ADD_INCLUDES}"
      COMPILE_OUTPUT_VARIABLE OUTPUT
      RUN_OUTPUT_VARIABLE RUN_OUTPUT)
    # if it did not compile make the return value fail code of 1
    if(NOT ${_var}_COMPILED)
      set(${_var}_EXITCODE 1)
      set(${_var}_EXITCODE 1 PARENT_SCOPE)
    endif()
    # if the return value was 0 then it worked
    if("${${_var}_EXITCODE}" EQUAL 0)
      set(${_var} 1 CACHE INTERNAL "Test ${_var}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "Success")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Performing ${_lang_textual} SOURCE FILE Test ${_var} succeeded with the following compile output:\n"
        "${OUTPUT}\n"
        "...and run output:\n"
        "${RUN_OUTPUT}\n"
        "Return value: ${${_var}}\n"
        "Source file was:\n${_source}\n")
    else()
      if(CMAKE_CROSSCOMPILING AND "${${_var}_EXITCODE}" MATCHES  "FAILED_TO_RUN")
        set(${_var} "${${_var}_EXITCODE}" PARENT_SCOPE)
      else()
        set(${_var} "" CACHE INTERNAL "Test ${_var}")
      endif()

      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "Failed")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Performing ${_lang_textual} SOURCE FILE Test ${_var} failed with the following compile output:\n"
        "${OUTPUT}\n"
        "...and run output:\n"
        "${RUN_OUTPUT}\n"
        "Return value: ${${_var}_EXITCODE}\n"
        "Source file was:\n${_source}\n")

    endif()
  endif()
endfunction()

cmake_policy(POP)
