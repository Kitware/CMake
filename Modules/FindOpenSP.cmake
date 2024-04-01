# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindOpenSP
----------

.. versionadded:: 3.25

Try to find the OpenSP library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpenSP_FOUND``
  True if (the requested version of) ``OpenSP`` is available

``OpenSP_VERSION``
  The version of ``OpenSP``

``OpenSP_VERSION_MAJOR``
  The major version of ``OpenSP``

``OpenSP_VERSION_MINOR``
  The minor version of ``OpenSP``

``OpenSP_VERSION_PATCH``
  The patch version of ``OpenSP``

``OpenSP_INCLUDE_DIRS``
  The include dirs of ``OpenSP`` with its headers

``OpenSP_LIBRARIES``
  The OpenSP library for use with target_link_libraries().
  This can be passed to target_link_libraries() instead of
  the :prop_tgt:`IMPORTED` ``OpenSP::OpenSP`` target

``OpenSP_MULTI_BYTE``
  True if ``SP_MULTI_BYTE`` was found to be defined in OpenSP's ``config.h``
  header file, which indicates that the ``OpenSP`` library was compiled with
  support for multi-byte characters. The consuming target needs to define the
  ``SP_MULTI_BYTE`` to match this value in order to avoid issues with character
  decoding.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines the :prop_tgt:`IMPORTED` target ``OpenSP::OpenSP``, if
OpenSP has been found.

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``OpenSP_INCLUDE_DIR``
  the OpenSP include directory

``OpenSP_LIBRARY``
  the absolute path of the osp library

#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
  pkg_check_modules(PC_OpenSP QUIET opensp)
endif ()

if (NOT OpenSP_INCLUDE_DIR)
  find_path(OpenSP_INCLUDE_DIR
    NAMES ParserEventGeneratorKit.h
    HINTS
    ${PC_OpenSP_INCLUDEDIRS}
    ${PC_OpenSP_INCLUDE_DIRS}
    PATH_SUFFIXES OpenSP opensp
    DOC "The OpenSP include directory"
    )
endif ()

if (NOT OpenSP_LIBRARY)
  find_library(OpenSP_LIBRARY_RELEASE
    NAMES osp libosp opensp libopensp sp133 libsp
    HINTS
    ${PC_OpenSP_LIBDIR}
    ${PC_OpenSP_LIBRARY_DIRS}
    )

  find_library(OpenSP_LIBRARY_DEBUG
    NAMES ospd libospd openspd libopenspd sp133d libspd
    HINTS
    ${PC_OpenSP_LIBDIR}
    ${PC_OpenSP_LIBRARY_DIRS}
    )

  include(SelectLibraryConfigurations)
  select_library_configurations(OpenSP)
endif ()

if (OpenSP_INCLUDE_DIR)
  if (EXISTS "${OpenSP_INCLUDE_DIR}/config.h")
    if (NOT OpenSP_VERSION)
      file(STRINGS "${OpenSP_INCLUDE_DIR}/config.h" opensp_version_str REGEX "^#define[\t ]+SP_VERSION[\t ]+\".*\"")
      string(REGEX REPLACE "^.*SP_VERSION[\t ]+\"([^\"]*)\".*$" "\\1" OpenSP_VERSION "${opensp_version_str}")
      unset(opensp_version_str)
    endif ()

    if (OpenSP_VERSION MATCHES [=[([0-9]+)\.([0-9]+)\.([0-9]+)]=])
      set(OpenSP_VERSION_MAJOR "${CMAKE_MATCH_1}")
      set(OpenSP_VERSION_MINOR "${CMAKE_MATCH_2}")
      set(OpenSP_VERSION_PATCH "${CMAKE_MATCH_3}")
    endif ()

    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(SP_MULTI_BYTE "${OpenSP_INCLUDE_DIR}/config.h" OpenSP_MULTI_BYTE)
  endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSP
  FOUND_VAR OpenSP_FOUND
  REQUIRED_VARS OpenSP_LIBRARY OpenSP_INCLUDE_DIR
  VERSION_VAR OpenSP_VERSION
  )

mark_as_advanced(OpenSP_INCLUDE_DIR OpenSP_LIBRARY OpenSP_MULTI_BYTE)

if (OpenSP_FOUND)
  set(OpenSP_INCLUDE_DIRS ${OpenSP_INCLUDE_DIR})
  if (NOT TARGET OpenSP::OpenSP)
    add_library(OpenSP::OpenSP UNKNOWN IMPORTED)
    if (EXISTS "${OpenSP_LIBRARY}")
      set_target_properties(OpenSP::OpenSP PROPERTIES
        IMPORTED_LOCATION "${OpenSP_LIBRARY}")
    endif ()
    set_target_properties(OpenSP::OpenSP PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${OpenSP_INCLUDE_DIRS}")

    if (OpenSP_LIBRARY_RELEASE)
      set_target_properties(OpenSP::OpenSP PROPERTIES
        IMPORTED_LOCATION_RELEASE "${OpenSP_LIBRARY_RELEASE}")
      set_property(TARGET OpenSP::OpenSP APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
    endif ()

    if (OpenSP_LIBRARY_DEBUG)
      set_target_properties(OpenSP::OpenSP PROPERTIES
        IMPORTED_LOCATION_DEBUG "${OpenSP_LIBRARY_DEBUG}")
      set_property(TARGET OpenSP::OpenSP APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
    endif ()
  endif ()
endif ()

include(FeatureSummary)
set_package_properties(OpenSP PROPERTIES
  URL "http://openjade.sourceforge.net/doc/index.htm"
  DESCRIPTION "An SGML System Conforming to International Standard ISO 8879"
  )

cmake_policy(POP)
