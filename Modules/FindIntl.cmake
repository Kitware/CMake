# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindIntl
--------

.. versionadded:: 3.2

Find the Gettext libintl headers and libraries.

This module reports information about the Gettext libintl
installation in several variables.

.. variable:: Intl_FOUND

  True if libintl is found.

.. variable:: Intl_INCLUDE_DIRS

  The directory containing the libintl headers.

.. variable:: Intl_LIBRARIES

  The intl libraries to be linked.

.. variable:: Intl_VERSION

  .. versionadded:: 3.21

  The version of intl found (x.y.z)

.. variable:: Intl_VERSION_MAJOR

  .. versionadded:: 3.21

  The major version of intl

.. variable:: Intl_VERSION_MINOR

  .. versionadded:: 3.21

  The minor version of intl

.. variable:: Intl_VERSION_PATCH

  .. versionadded:: 3.21

  The patch version of intl

.. versionadded:: 3.20
  This module defines :prop_tgt:`IMPORTED` target ``Intl::Intl``.

The following cache variables may also be set:

.. variable:: Intl_INCLUDE_DIR

  The directory containing the libintl headers

.. variable:: Intl_LIBRARY

  The libintl library (if any)

.. variable:: Intl_HAVE_GETTEXT_BUILTIN

  .. versionadded:: 3.20

  True if gettext is in the C library

.. variable:: Intl_HAVE_DCGETTEXT_BUILTIN

  .. versionadded:: 3.20

  True if dcgettext is in the C library

.. variable:: Intl_IS_BUILTIN

  .. versionadded:: 3.20

  whether intl is a part of the C library determined from the result of
  Intl_HAVE_GETTEXT_BUILTIN and Intl_HAVE_DCGETTEXT_BUILTIN

.. note::
  On some platforms, such as Linux with GNU libc, the gettext
  functions are present in the C standard library and libintl
  is not required.  ``Intl_LIBRARIES`` will be empty in this
  case.

.. note::
  Some libintl implementations don't embed the version number in their header files.
  In this case the variables ``Intl_VERSION*`` will be empty.

.. note::
  If you wish to use the Gettext tools (``msgmerge``,
  ``msgfmt``, etc.), use :module:`FindGettext`.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/CMakePushCheckState.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CheckSymbolExists.cmake)

# Check if we have libintl is a part of libc
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_QUIET TRUE)
check_symbol_exists(gettext libintl.h Intl_HAVE_GETTEXT_BUILTIN)
check_symbol_exists(dcgettext libintl.h Intl_HAVE_DCGETTEXT_BUILTIN) # redundant check
cmake_pop_check_state()

if(Intl_HAVE_GETTEXT_BUILTIN AND Intl_HAVE_DCGETTEXT_BUILTIN)
  set(Intl_IS_BUILTIN TRUE)
else()
  set(Intl_IS_BUILTIN FALSE)
endif()

# Find include directory
find_path(Intl_INCLUDE_DIR
          NAMES "libintl.h"
          DOC "libintl include directory")
mark_as_advanced(Intl_INCLUDE_DIR)

# Find all Intl libraries
if(NOT Intl_IS_BUILTIN)
  set(Intl_LIBRARY_NAMES "intl" "libintl")
else()
  set(Intl_LIBRARY_NAMES "c")
endif()

find_library(Intl_LIBRARY ${Intl_LIBRARY_NAMES} NAMES_PER_DIR
              DOC "intl library (potentially the C library)")
mark_as_advanced(Intl_LIBRARY)

# NOTE: glibc's libintl.h does not define LIBINTL_VERSION
if(Intl_INCLUDE_DIR AND NOT Intl_IS_BUILTIN)
  file(STRINGS ${Intl_INCLUDE_DIR}/libintl.h Intl_VERSION_DEFINE REGEX "LIBINTL_VERSION (.*)")

  if(Intl_VERSION_DEFINE MATCHES "(0x[A-Fa-f0-9]+)")
    set(Intl_VERSION_NUMBER "${CMAKE_MATCH_1}")
    # encoding -> version number: (major<<16) + (minor<<8) + patch
    math(EXPR Intl_VERSION_MAJOR "${Intl_VERSION_NUMBER} >> 16" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR Intl_VERSION_MINOR "(${Intl_VERSION_NUMBER} - (${Intl_VERSION_MAJOR} << 16)) >> 8" OUTPUT_FORMAT HEXADECIMAL)
    math(EXPR Intl_VERSION_PATCH "${Intl_VERSION_NUMBER} - ((${Intl_VERSION_MAJOR} << 16) + (${Intl_VERSION_MINOR} << 8))" OUTPUT_FORMAT HEXADECIMAL)

    math(EXPR Intl_VERSION_MAJOR "${Intl_VERSION_MAJOR}" OUTPUT_FORMAT DECIMAL)
    math(EXPR Intl_VERSION_MINOR "${Intl_VERSION_MINOR}" OUTPUT_FORMAT DECIMAL)
    math(EXPR Intl_VERSION_PATCH "${Intl_VERSION_PATCH}" OUTPUT_FORMAT DECIMAL)
    set(Intl_VERSION "${Intl_VERSION_MAJOR}.${Intl_VERSION_MINOR}.${Intl_VERSION_PATCH}")
  endif()

  unset(Intl_VERSION_DEFINE)
  unset(Intl_VERSION_NUMBER)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Intl
                                  FOUND_VAR Intl_FOUND
                                  REQUIRED_VARS Intl_LIBRARY Intl_INCLUDE_DIR
                                  VERSION_VAR Intl_VERSION
                                  FAIL_MESSAGE "Failed to find Gettext libintl")

if(Intl_FOUND)
  set(Intl_INCLUDE_DIRS "${Intl_INCLUDE_DIR}")
  set(Intl_LIBRARIES "${Intl_LIBRARY}")
  if(NOT TARGET Intl::Intl)
    add_library(Intl::Intl INTERFACE IMPORTED)
    set_target_properties(Intl::Intl PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Intl_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${Intl_LIBRARIES}")
  endif()
endif()
