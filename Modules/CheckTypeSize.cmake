# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckTypeSize
-------------

This module provides a command to check the size of a C/C++ type or expression.

Load this module in a CMake project with:

.. code-block:: cmake

  include(CheckTypeSize)

Commands
^^^^^^^^

This module provides the following command:

.. command:: check_type_size

  Checks once whether the C/C++ type or expression exists and determines its
  size:

  .. code-block:: cmake

    check_type_size(<type> <variable> [BUILTIN_TYPES_ONLY] [LANGUAGE <language>])

  The arguments are:

  ``<type>``
    The type or expression being checked.

  ``<variable>``
    The name of the variable and a prefix used for storing the check results.

  ``BUILTIN_TYPES_ONLY``
    If given, only compiler-builtin types will be supported in the check.
    If *not* given, the command checks for common headers ``<sys/types.h>``,
    ``<stdint.h>``, and ``<stddef.h>``, and saves results in
    ``HAVE_SYS_TYPES_H``, ``HAVE_STDINT_H``, and ``HAVE_STDDEF_H`` internal
    cache variables.  The type size check automatically includes the available
    headers, thus supporting checks of types defined in the headers.

  ``LANGUAGE <language>``
    Uses the ``<language>`` compiler to perform the check.
    Acceptable values are ``C`` and ``CXX``.
    If not specified, it defaults to ``C``.

  .. rubric:: Result Variables

  Results are reported in the following variables:

  ``HAVE_<variable>``
    Internal cache variable that holds a boolean true or false value
    indicating whether the type or expression ``<type>`` exists.

  ``<variable>``
    Internal cache variable that holds one of the following values:

    ``<size>``
      If the type or expression exists, it will have a non-zero size
      ``<size>`` in bytes.

    ``0``
      When type has architecture-dependent size;  This may occur when
      :variable:`CMAKE_OSX_ARCHITECTURES` has multiple architectures.
      In this case ``<variable>_CODE`` contains preprocessor tests
      mapping from each architecture macro to the corresponding type size.
      The list of architecture macros is stored in ``<variable>_KEYS``,
      and the value for each key is stored in ``<variable>-<key>``.

    "" (empty string)
      When type or expression does not exist.

  ``<variable>_CODE``
    CMake variable that holds preprocessor code to define the macro
    ``<variable>`` to the size of the type, or to leave the macro undefined
    if the type does not exist.

  Despite the name of this command, it may also be used to determine the size
  of more complex expressions.  For example, to check the size of a struct
  member:

  .. code-block:: cmake

    check_type_size("((struct something*)0)->member" SIZEOF_MEMBER)

  .. rubric:: Variables Affecting the Check

  The following variables may be set before calling this command to modify
  the way the check is run:

  .. include:: /module/include/CMAKE_REQUIRED_FLAGS.rst

  .. include:: /module/include/CMAKE_REQUIRED_DEFINITIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_INCLUDES.rst

  .. include:: /module/include/CMAKE_REQUIRED_LINK_OPTIONS.rst

  .. include:: /module/include/CMAKE_REQUIRED_LIBRARIES.rst

  .. include:: /module/include/CMAKE_REQUIRED_LINK_DIRECTORIES.rst

  .. include:: /module/include/CMAKE_REQUIRED_QUIET.rst

  ``CMAKE_EXTRA_INCLUDE_FILES``
    A :ref:`semicolon-separated list <CMake Language Lists>` of extra header
    files to include when performing the check.

Examples
^^^^^^^^

Consider the code:

.. code-block:: cmake

  include(CheckTypeSize)

  # Check for size of long.
  check_type_size(long SIZEOF_LONG)
  message("HAVE_SIZEOF_LONG: ${HAVE_SIZEOF_LONG}")
  message("SIZEOF_LONG: ${SIZEOF_LONG}")
  message("SIZEOF_LONG_CODE: ${SIZEOF_LONG_CODE}")

On a 64-bit architecture, the output may look something like this::

  HAVE_SIZEOF_LONG: TRUE
  SIZEOF_LONG: 8
  SIZEOF_LONG_CODE: #define SIZEOF_LONG 8

On Apple platforms, when :variable:`CMAKE_OSX_ARCHITECTURES` has multiple
architectures, types may have architecture-dependent sizes.
For example, with the code

.. code-block:: cmake

  include(CheckTypeSize)

  check_type_size(long SIZEOF_LONG)
  message("HAVE_SIZEOF_LONG: ${HAVE_SIZEOF_LONG}")
  message("SIZEOF_LONG: ${SIZEOF_LONG}")
  foreach(key IN LISTS SIZE_OF_LONG_KEYS)
    message("key: ${key}")
    message("value: ${SIZE_OF_LONG-${key}}")
  endforeach()
  message("SIZEOF_LONG_CODE:
  ${SIZEOF_LONG_CODE}")

the result may be::

  HAVE_SIZEOF_LONG: TRUE
  SIZEOF_LONG: 0
  key: __i386
  value: 4
  key: __x86_64
  value: 8
  SIZEOF_LONG_CODE:
  #if defined(__i386)
  # define SIZE_OF_LONG 4
  #elif defined(__x86_64)
  # define SIZE_OF_LONG 8
  #else
  # error SIZE_OF_LONG unknown
  #endif
#]=======================================================================]

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

get_filename_component(__check_type_size_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)

include_guard(GLOBAL)

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

#-----------------------------------------------------------------------------
# Helper function.  DO NOT CALL DIRECTLY.
function(__check_type_size_impl type var map builtin language)
  if(NOT CMAKE_REQUIRED_QUIET)
    message(CHECK_START "Check size of ${type}")
  endif()

  # Perform language check
  string(MAKE_C_IDENTIFIER ${var} _var_escaped)
  if(language STREQUAL "C")
    set(src ${_var_escaped}.c)
  elseif(language STREQUAL "CXX")
    set(src ${_var_escaped}.cpp)
  else()
    message(FATAL_ERROR "Unknown language:\n  ${language}\nSupported languages: C, CXX.\n")
  endif()

  # Include header files.
  set(headers)
  if(builtin)
    if(language STREQUAL "CXX" AND type MATCHES "^std::")
      if(HAVE_SYS_TYPES_H)
        string(APPEND headers "#include <sys/types.h>\n")
      endif()
      if(HAVE_CSTDINT)
        string(APPEND headers "#include <cstdint>\n")
      endif()
      if(HAVE_CSTDDEF)
        string(APPEND headers "#include <cstddef>\n")
      endif()
    else()
      if(HAVE_SYS_TYPES_H)
        string(APPEND headers "#include <sys/types.h>\n")
      endif()
      if(HAVE_STDINT_H)
        string(APPEND headers "#include <stdint.h>\n")
      endif()
      if(HAVE_STDDEF_H)
        string(APPEND headers "#include <stddef.h>\n")
      endif()
    endif()
  endif()
  foreach(h ${CMAKE_EXTRA_INCLUDE_FILES})
    string(APPEND headers "#include \"${h}\"\n")
  endforeach()

  if(CMAKE_REQUIRED_LINK_DIRECTORIES)
    set(_CTS_LINK_DIRECTORIES
      "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
  else()
    set(_CTS_LINK_DIRECTORIES)
  endif()

  # Perform the check.
  set(bin ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${var}.bin)
  file(READ ${__check_type_size_dir}/CheckTypeSize.c.in src_content)
  string(CONFIGURE "${src_content}" src_content @ONLY)
  try_compile(HAVE_${var} SOURCE_FROM_VAR "${src}" src_content
    COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
    LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS}
    LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES}
    CMAKE_FLAGS
      "-DCOMPILE_DEFINITIONS:STRING=${CMAKE_REQUIRED_FLAGS}"
      "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}"
      "${_CTS_LINK_DIRECTORIES}"
    COPY_FILE ${bin}
    )
  unset(_CTS_LINK_DIRECTORIES)

  if(HAVE_${var})
    # The check compiled.  Load information from the binary.
    file(STRINGS ${bin} strings LIMIT_COUNT 10 REGEX "INFO:size")

    # Parse the information strings.
    set(regex_size ".*INFO:size\\[0*([^]]*)\\].*")
    set(regex_key " key\\[([^]]*)\\]")
    set(keys)
    set(code)
    set(mismatch)
    set(first 1)
    foreach(info ${strings})
      if("${info}" MATCHES "${regex_size}")
        # Get the type size.
        set(size "${CMAKE_MATCH_1}")
        if(first)
          set(${var} ${size})
        elseif(NOT "${size}" STREQUAL "${${var}}")
          set(mismatch 1)
        endif()
        set(first 0)

        # Get the architecture map key.
        string(REGEX MATCH   "${regex_key}"       key "${info}")
        string(REGEX REPLACE "${regex_key}" "\\1" key "${key}")
        if(key)
          string(APPEND code "\nset(${var}-${key} \"${size}\")")
          list(APPEND keys ${key})
        endif()
      endif()
    endforeach()

    # Update the architecture-to-size map.
    if(mismatch AND keys)
      configure_file(${__check_type_size_dir}/CheckTypeSizeMap.cmake.in ${map} @ONLY)
      set(${var} 0)
    else()
      file(REMOVE ${map})
    endif()

    if(mismatch AND NOT keys)
      message(SEND_ERROR "CHECK_TYPE_SIZE found different results, consider setting CMAKE_OSX_ARCHITECTURES or CMAKE_TRY_COMPILE_OSX_ARCHITECTURES to one or no architecture !")
    endif()

    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_PASS "done")
    endif()
    set(${var} "${${var}}" CACHE INTERNAL "CHECK_TYPE_SIZE: sizeof(${type})")
  else()
    # The check failed to compile.
    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_FAIL "failed")
    endif()
    set(${var} "" CACHE INTERNAL "CHECK_TYPE_SIZE: ${type} unknown")
    file(REMOVE ${map})
  endif()
endfunction()

#-----------------------------------------------------------------------------
macro(CHECK_TYPE_SIZE TYPE VARIABLE)
  # parse arguments
  unset(doing)
  foreach(arg ${ARGN})
    if("x${arg}" STREQUAL "xBUILTIN_TYPES_ONLY")
      set(_CHECK_TYPE_SIZE_${arg} 1)
      unset(doing)
    elseif("x${arg}" STREQUAL "xLANGUAGE") # change to MATCHES for more keys
      set(doing "${arg}")
      set(_CHECK_TYPE_SIZE_${doing} "")
    elseif("x${doing}" STREQUAL "xLANGUAGE")
      set(_CHECK_TYPE_SIZE_${doing} "${arg}")
      unset(doing)
    else()
      message(FATAL_ERROR "Unknown argument:\n  ${arg}\n")
    endif()
  endforeach()
  if("x${doing}" MATCHES "^x(LANGUAGE)$")
    message(FATAL_ERROR "Missing argument:\n  ${doing} arguments requires a value\n")
  endif()
  if(DEFINED _CHECK_TYPE_SIZE_LANGUAGE)
    if(NOT "x${_CHECK_TYPE_SIZE_LANGUAGE}" MATCHES "^x(C|CXX)$")
      message(FATAL_ERROR "Unknown language:\n  ${_CHECK_TYPE_SIZE_LANGUAGE}.\nSupported languages: C, CXX.\n")
    endif()
    set(_language ${_CHECK_TYPE_SIZE_LANGUAGE})
  else()
    set(_language C)
  endif()

  # Optionally check for standard headers.
  if(_CHECK_TYPE_SIZE_BUILTIN_TYPES_ONLY)
    set(_builtin 0)
  else()
    set(_builtin 1)
    if(_language STREQUAL "C")
      check_include_file(sys/types.h HAVE_SYS_TYPES_H)
      check_include_file(stdint.h HAVE_STDINT_H)
      check_include_file(stddef.h HAVE_STDDEF_H)
    elseif(_language STREQUAL "CXX")
      check_include_file_cxx(sys/types.h HAVE_SYS_TYPES_H)
      if("${TYPE}" MATCHES "^std::")
        check_include_file_cxx(cstdint HAVE_CSTDINT)
        check_include_file_cxx(cstddef HAVE_CSTDDEF)
      else()
        check_include_file_cxx(stdint.h HAVE_STDINT_H)
        check_include_file_cxx(stddef.h HAVE_STDDEF_H)
      endif()
    endif()
  endif()
  unset(_CHECK_TYPE_SIZE_BUILTIN_TYPES_ONLY)
  unset(_CHECK_TYPE_SIZE_LANGUAGE)

  # Compute or load the size or size map.
  set(${VARIABLE}_KEYS)
  set(_map_file ${CMAKE_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/CheckTypeSize/${VARIABLE}.cmake)
  if(NOT DEFINED HAVE_${VARIABLE})
    __check_type_size_impl(${TYPE} ${VARIABLE} ${_map_file} ${_builtin} ${_language})
  endif()
  include(${_map_file} OPTIONAL)
  set(_map_file)
  set(_builtin)

  # Create preprocessor code.
  if(${VARIABLE}_KEYS)
    set(${VARIABLE}_CODE)
    set(_if if)
    foreach(key ${${VARIABLE}_KEYS})
      string(APPEND ${VARIABLE}_CODE "#${_if} defined(${key})\n# define ${VARIABLE} ${${VARIABLE}-${key}}\n")
      set(_if elif)
    endforeach()
    string(APPEND ${VARIABLE}_CODE "#else\n# error ${VARIABLE} unknown\n#endif")
    set(_if)
  elseif(${VARIABLE})
    set(${VARIABLE}_CODE "#define ${VARIABLE} ${${VARIABLE}}")
  else()
    set(${VARIABLE}_CODE "/* #undef ${VARIABLE} */")
  endif()
endmacro()

#-----------------------------------------------------------------------------
endblock()
