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

.. versionadded:: 3.20
  This module defines :prop_tgt:`IMPORTED` target ``Intl::Intl``.

The following cache variables may also be set:

.. variable:: Intl_INCLUDE_DIR

  The directory containing the libintl headers

.. variable:: Intl_LIBRARY

  The libintl library (if any)

.. variable:: Intl_IS_BUILT_IN

  .. versionadded:: 3.20

  whether ``intl`` is a part of the C library.

.. note::
  On some platforms, such as Linux with GNU libc, the gettext
  functions are present in the C standard library and libintl
  is not required.  ``Intl_LIBRARIES`` will be empty in this
  case.

.. note::
  If you wish to use the Gettext tools (``msgmerge``,
  ``msgfmt``, etc.), use :module:`FindGettext`.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/CMakePushCheckState.cmake)
if(CMAKE_C_COMPILER_LOADED)
  include(${CMAKE_CURRENT_LIST_DIR}/CheckCSourceCompiles.cmake)
elseif(CMAKE_CXX_COMPILER_LOADED)
  include(${CMAKE_CURRENT_LIST_DIR}/CheckCXXSourceCompiles.cmake)
else()
  # If neither C nor CXX are loaded, implicit intl makes no sense.
  set(Intl_IS_BUILT_IN FALSE)
endif()

# Check if Intl is built in to the C library.
if(NOT DEFINED Intl_IS_BUILT_IN)
  if(NOT DEFINED Intl_INCLUDE_DIR AND NOT DEFINED Intl_LIBRARY)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET TRUE)
    set(Intl_IMPLICIT_TEST_CODE [[
#include <libintl.h>
int main(void) {
  gettext("");
  dgettext("", "");
  dcgettext("", "", 0);
  return 0;
}
]])
    if(CMAKE_C_COMPILER_LOADED)
      check_c_source_compiles("${Intl_IMPLICIT_TEST_CODE}" Intl_IS_BUILT_IN)
    else()
      check_cxx_source_compiles("${Intl_IMPLICIT_TEST_CODE}" Intl_IS_BUILT_IN)
    endif()
    cmake_pop_check_state()
  else()
    set(Intl_IS_BUILT_IN FALSE)
  endif()
endif()

set(_Intl_REQUIRED_VARS)
if(Intl_IS_BUILT_IN)
  set(_Intl_REQUIRED_VARS _Intl_IS_BUILT_IN_MSG)
  set(_Intl_IS_BUILT_IN_MSG "built in to C library")
else()
  set(_Intl_REQUIRED_VARS Intl_LIBRARY Intl_INCLUDE_DIR)

  find_path(Intl_INCLUDE_DIR
            NAMES "libintl.h"
            DOC "libintl include directory")
  mark_as_advanced(Intl_INCLUDE_DIR)

  find_library(Intl_LIBRARY "intl" "libintl" NAMES_PER_DIR
    DOC "libintl libraries (if not in the C library)")
  mark_as_advanced(Intl_LIBRARY)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Intl
                                  FOUND_VAR Intl_FOUND
                                  REQUIRED_VARS ${_Intl_REQUIRED_VARS}
                                  FAIL_MESSAGE "Failed to find Gettext libintl")
unset(_Intl_REQUIRED_VARS)
unset(_Intl_IS_BUILT_IN_MSG)

if(Intl_FOUND)
  if(Intl_IS_BUILT_IN)
    set(Intl_INCLUDE_DIRS "")
    set(Intl_LIBRARIES "")
  else()
    set(Intl_INCLUDE_DIRS "${Intl_INCLUDE_DIR}")
    set(Intl_LIBRARIES "${Intl_LIBRARY}")
  endif()
  if(NOT TARGET Intl::Intl)
    add_library(Intl::Intl INTERFACE IMPORTED)
    set_target_properties(Intl::Intl PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Intl_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${Intl_LIBRARIES}")
  endif()
endif()
