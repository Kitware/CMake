# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindIntl
--------

.. versionadded:: 3.2

Find the Gettext libintl headers and libraries.

This module reports information about the Gettext libintl
installation in several variables.  General variables::

  Intl_FOUND - true if the libintl headers and libraries were found
  Intl_INCLUDE_DIRS - the directory containing the libintl headers
  Intl_LIBRARIES - libintl libraries to be linked

.. versionadded:: 3.20
  This module defines :prop_tgt:`IMPORTED` target ``Intl::Intl``.

The following cache variables may also be set::

  Intl_INCLUDE_DIR - the directory containing the libintl headers
  Intl_LIBRARY - the libintl library (if any)
  Intl_HAVE_GETTEXT_BUILTIN - check if gettext is in the C library
  Intl_HAVE_DCGETTEXT_BUILTIN - check if dcgettext is in the C library
  Intl_IS_BUILTIN - whether intl is a part of the C library determined
      from the result of Intl_HAVE_GETTEXT_BUILTIN and Intl_HAVE_DCGETTEXT_BUILTIN

.. versionadded:: 3.20
  Added the ``Intl_HAVE_GETTEXT_BUILTIN``, ``Intl_HAVE_DCGETTEXT_BUILTIN`` and
  ``Intl_IS_BUILTIN`` variables.

.. note::
  On some platforms, such as Linux with GNU libc, the gettext
  functions are present in the C standard library and libintl
  is not required.  ``Intl_LIBRARIES`` will be empty in this
  case.

.. note::
  If you wish to use the Gettext tools (``msgmerge``,
  ``msgfmt``, etc.), use :module:`FindGettext`.
#]=======================================================================]


# Written by Roger Leigh <rleigh@codelibre.net>

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
set(Intl_REQUIRED_VARS)
if(NOT Intl_IS_BUILTIN)
  find_library(Intl_LIBRARY "intl" "libintl" NAMES_PER_DIR
    DOC "libintl libraries (if not in the C library)")
  mark_as_advanced(Intl_LIBRARY)
  list(APPEND Intl_REQUIRED_VARS Intl_LIBRARY)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Intl
                                  FOUND_VAR Intl_FOUND
                                  REQUIRED_VARS Intl_INCLUDE_DIR ${Intl_REQUIRED_VARS}
                                  FAIL_MESSAGE "Failed to find Gettext libintl")
unset(Intl_REQUIRED_VARS)

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
