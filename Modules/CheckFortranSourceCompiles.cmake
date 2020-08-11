# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckFortranSourceCompiles
--------------------------

Check if given Fortran source compiles and links into an executable.

.. command:: check_fortran_source_compiles

  .. code-block:: cmake

    check_fortran_source_compiles(<code> <resultVar>
        [FAIL_REGEX <regex>...]
        [SRC_EXT <extension>]
    )

  Checks that the source supplied in ``<code>`` can be compiled as a Fortran
  source file and linked as an executable. The ``<code>`` must be a Fortran program
  containing at least an ``end`` statement--for example:

  .. code-block:: cmake

    check_fortran_source_compiles("character :: b; error stop b; end" F2018ESTOPOK SRC_EXT F90)

  This command can help avoid costly build processes when a compiler lacks support
  for a necessary feature, or a particular vendor library is not compatible with
  the Fortran compiler version being used. This generate-time check may advise the
  user of such before the main build process. See also the
  :command:`check_fortran_source_runs` command to actually run the compiled code.

  The result will be stored in the internal cache
  variable ``<resultVar>``, with a boolean true value for success and boolean
  false for failure.

  If ``FAIL_REGEX`` is provided, then failure is determined by checking
  if anything in the output matches any of the specified regular expressions.

  By default, the test source file will be given a ``.F`` file extension. The
  ``SRC_EXT`` option can be used to override this with ``.<extension>`` instead--
  ``.F90`` is a typical choice.

  The underlying check is performed by the :command:`try_compile` command. The
  compile and link commands can be influenced by setting any of the following
  variables prior to calling ``check_fortran_source_compiles()``:

  ``CMAKE_REQUIRED_FLAGS``
    Additional flags to pass to the compiler. Note that the contents of
    :variable:`CMAKE_Fortran_FLAGS <CMAKE_<LANG>_FLAGS>` and its associated
    configuration-specific variable are automatically added to the compiler
    command before the contents of ``CMAKE_REQUIRED_FLAGS``.

  ``CMAKE_REQUIRED_DEFINITIONS``
    A :ref:`;-list <CMake Language Lists>` of compiler definitions of the form
    ``-DFOO`` or ``-DFOO=bar``. A definition for the name specified by
    ``<resultVar>`` will also be added automatically.

  ``CMAKE_REQUIRED_INCLUDES``
    A :ref:`;-list <CMake Language Lists>` of header search paths to pass to
    the compiler. These will be the only header search paths used by
    ``try_compile()``, i.e. the contents of the :prop_dir:`INCLUDE_DIRECTORIES`
    directory property will be ignored.

  ``CMAKE_REQUIRED_LINK_OPTIONS``
    A :ref:`;-list <CMake Language Lists>` of options to add to the link
    command (see :command:`try_compile` for further details).

  ``CMAKE_REQUIRED_LIBRARIES``
    A :ref:`;-list <CMake Language Lists>` of libraries to add to the link
    command. These can be the name of system libraries or they can be
    :ref:`Imported Targets <Imported Targets>` (see :command:`try_compile` for
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

macro(CHECK_Fortran_SOURCE_COMPILES SOURCE VAR)
  if(NOT DEFINED "${VAR}")
    set(_FAIL_REGEX)
    set(_SRC_EXT)
    set(_key)
    foreach(arg ${ARGN})
      if("${arg}" MATCHES "^(FAIL_REGEX|SRC_EXT)$")
        set(_key "${arg}")
      elseif(_key)
        list(APPEND _${_key} "${arg}")
      else()
        message(FATAL_ERROR "Unknown argument:\n  ${arg}\n")
      endif()
    endforeach()
    if(NOT _SRC_EXT)
      set(_SRC_EXT F)
    endif()
    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_LINK_OPTIONS)
    endif()
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_LIBRARIES
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_LIBRARIES)
    endif()
    if(CMAKE_REQUIRED_INCLUDES)
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CHECK_Fortran_SOURCE_COMPILES_ADD_INCLUDES)
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.${_SRC_EXT}"
      "${SOURCE}\n")

    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Performing Test ${VAR}")
    endif()
    try_compile(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.${_SRC_EXT}
      COMPILE_DEFINITIONS -D${VAR} ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_Fortran_SOURCE_COMPILES_ADD_LINK_OPTIONS}
      ${CHECK_Fortran_SOURCE_COMPILES_ADD_LIBRARIES}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${CMAKE_REQUIRED_FLAGS}
      "${CHECK_Fortran_SOURCE_COMPILES_ADD_INCLUDES}"
      OUTPUT_VARIABLE OUTPUT)

    foreach(_regex ${_FAIL_REGEX})
      if("${OUTPUT}" MATCHES "${_regex}")
        set(${VAR} 0)
      endif()
    endforeach()

    if(${VAR})
      set(${VAR} 1 CACHE INTERNAL "Test ${VAR}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "Success")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Performing Fortran SOURCE FILE Test ${VAR} succeeded with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    else()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "Failed")
      endif()
      set(${VAR} "" CACHE INTERNAL "Test ${VAR}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Performing Fortran SOURCE FILE Test ${VAR} failed with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    endif()
  endif()
endmacro()
