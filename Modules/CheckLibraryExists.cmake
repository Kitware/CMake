# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckLibraryExists
------------------

Check once if the function exists in system or specified library.

.. command:: check_library_exists

  .. code-block:: cmake

    check_library_exists(<library> <function> <location> <variable>)

  Check that the library ``<library>`` exists in the given location
  ``<location>`` and has the specified ``<function>``. The result is stored in
  an internal cache variable ``<variable>``. If ``<location>`` is empty string,
  default directories are searched.

Prefer using :module:`CheckSymbolExists` or :module:`CheckSourceCompiles`
instead of this module for more robust detection if a function is available in
a library.

The following variables may be set before calling this macro to modify
the way the check is run:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

Examples
^^^^^^^^

This module can be useful for performing so-called sanity checks to verify that
the specified library provides the expected functionality and is indeed the
correct one being located.

For example, to check if the ``curl`` library exists in the default paths and
has the ``curl_easy_perform`` function:

.. code-block:: cmake

  include(CheckLibraryExists)
  check_library_exists(curl curl_easy_perform "" HAVE_LIBRARY_CURL)

To check if library exists in specific non-standard location and has a specified
function:

.. code-block:: cmake

  include(CheckLibraryExists)
  check_library_exists(curl curl_easy_perform "/opt/curl/lib" HAVE_LIBRARY_CURL)

Also :ref:`IMPORTED library <add_library imported libraries>` (for example,
from the ``find_package()`` call) can be used:

.. code-block:: cmake

  find_package(CURL)

  # ...

  if(TARGET CURL::libcurl)
    include(CheckLibraryExists)
    check_library_exists(CURL::libcurl curl_easy_perform "" HAVE_LIBRARY_CURL)
  endif()
#]=======================================================================]

include_guard(GLOBAL)

macro(CHECK_LIBRARY_EXISTS LIBRARY FUNCTION LOCATION VARIABLE)
  if(NOT DEFINED "${VARIABLE}")
    set(MACRO_CHECK_LIBRARY_EXISTS_DEFINITION
      "-DCHECK_FUNCTION_EXISTS=${FUNCTION} ${CMAKE_REQUIRED_FLAGS}")
    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Looking for ${FUNCTION} in ${LIBRARY}")
    endif()
    set(CHECK_LIBRARY_EXISTS_LINK_OPTIONS)
    if(CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_LIBRARY_EXISTS_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    endif()
    set(CHECK_LIBRARY_EXISTS_LIBRARIES ${LIBRARY})
    if(CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_LIBRARY_EXISTS_LIBRARIES
        ${CHECK_LIBRARY_EXISTS_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES})
    endif()
    if(CMAKE_REQUIRED_LINK_DIRECTORIES)
      set(_CLE_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${LOCATION};${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CLE_LINK_DIRECTORIES "-DLINK_DIRECTORIES:STRING=${LOCATION}")
    endif()

    if(CMAKE_C_COMPILER_LOADED)
      set(_cle_source CheckFunctionExists.c)
    elseif(CMAKE_CXX_COMPILER_LOADED)
      set(_cle_source CheckFunctionExists.cxx)
    else()
      message(FATAL_ERROR "CHECK_FUNCTION_EXISTS needs either C or CXX language enabled")
    endif()

    try_compile(${VARIABLE}
      SOURCE_FROM_FILE "${_cle_source}" "${CMAKE_ROOT}/Modules/CheckFunctionExists.c"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_LIBRARY_EXISTS_LINK_OPTIONS}
      LINK_LIBRARIES ${CHECK_LIBRARY_EXISTS_LIBRARIES}
      CMAKE_FLAGS
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_LIBRARY_EXISTS_DEFINITION}
      "${_CLE_LINK_DIRECTORIES}"
      )
    unset(_cle_source)
    unset(_CLE_LINK_DIRECTORIES)

    if(${VARIABLE})
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "found")
      endif()
      set(${VARIABLE} 1 CACHE INTERNAL "Have library ${LIBRARY}")
    else()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "not found")
      endif()
      set(${VARIABLE} "" CACHE INTERNAL "Have library ${LIBRARY}")
    endif()
  endif()
endmacro()
