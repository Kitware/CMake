# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckCSourceRuns
----------------

Check if given C source compiles and links into an executable and can
subsequently be run.

.. command:: check_c_source_runs

  .. code-block:: cmake

    check_c_source_runs(<code> <resultVar>)

  Check that the source supplied in ``<code>`` can be compiled as a C source
  file, linked as an executable and then run. The ``<code>`` must contain at
  least a ``main()`` function. If the ``<code>`` could be built and run
  successfully, the internal cache variable specified by ``<resultVar>`` will
  be set to 1, otherwise it will be set to an value that evaluates to boolean
  false (e.g. an empty string or an error message).

  The underlying check is performed by the :command:`try_run` command. The
  compile and link commands can be influenced by setting any of the following
  variables prior to calling ``check_c_source_runs()``:

  ``CMAKE_REQUIRED_FLAGS``
    Additional flags to pass to the compiler. Note that the contents of
    :variable:`CMAKE_C_FLAGS <CMAKE_<LANG>_FLAGS>` and its associated
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

macro(CHECK_C_SOURCE_RUNS SOURCE VAR)
  if(NOT DEFINED "${VAR}")
    set(MACRO_CHECK_FUNCTION_DEFINITIONS
      "-D${VAR} ${CMAKE_REQUIRED_FLAGS}")
    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_C_SOURCE_COMPILES_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_C_SOURCE_COMPILES_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_C_SOURCE_COMPILES_ADD_INCLUDES)
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c"
      "${SOURCE}\n")

    if(NOT CMAKE_REQUIRED_QUIET)
      message(STATUS "Performing Test ${VAR}")
    endif()
    try_run(${VAR}_EXITCODE ${VAR}_COMPILED
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_C_SOURCE_COMPILES_ADD_LINK_OPTIONS}
      ${CHECK_C_SOURCE_COMPILES_ADD_LIBRARIES}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
      -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
      "${CHECK_C_SOURCE_COMPILES_ADD_INCLUDES}"
      COMPILE_OUTPUT_VARIABLE OUTPUT
      RUN_OUTPUT_VARIABLE RUN_OUTPUT)
    # if it did not compile make the return value fail code of 1
    if(NOT ${VAR}_COMPILED)
      set(${VAR}_EXITCODE 1)
    endif()
    # if the return value was 0 then it worked
    if("${${VAR}_EXITCODE}" EQUAL 0)
      set(${VAR} 1 CACHE INTERNAL "Test ${VAR}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(STATUS "Performing Test ${VAR} - Success")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Performing C SOURCE FILE Test ${VAR} succeeded with the following compile output:\n"
        "${OUTPUT}\n"
        "...and run output:\n"
        "${RUN_OUTPUT}\n"
        "Return value: ${${VAR}}\n"
        "Source file was:\n${SOURCE}\n")
    else()
      if(CMAKE_CROSSCOMPILING AND "${${VAR}_EXITCODE}" MATCHES  "FAILED_TO_RUN")
        set(${VAR} "${${VAR}_EXITCODE}")
      else()
        set(${VAR} "" CACHE INTERNAL "Test ${VAR}")
      endif()

      if(NOT CMAKE_REQUIRED_QUIET)
        message(STATUS "Performing Test ${VAR} - Failed")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Performing C SOURCE FILE Test ${VAR} failed with the following compile output:\n"
        "${OUTPUT}\n"
        "...and run output:\n"
        "${RUN_OUTPUT}\n"
        "Return value: ${${VAR}_EXITCODE}\n"
        "Source file was:\n${SOURCE}\n")

    endif()
  endif()
endmacro()
