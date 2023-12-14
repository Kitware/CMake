# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindFreetype
------------

Find the FreeType font renderer includes and library.

Imported Targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.10

This module defines the following :prop_tgt:`IMPORTED` target:

``Freetype::Freetype``
  The Freetype ``freetype`` library, if found

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``FREETYPE_FOUND``
  true if the Freetype headers and libraries were found
``FREETYPE_INCLUDE_DIRS``
  directories containing the Freetype headers. This is the
  concatenation of the variables:

  ``FREETYPE_INCLUDE_DIR_ft2build``
    directory holding the main Freetype API configuration header
  ``FREETYPE_INCLUDE_DIR_freetype2``
    directory holding Freetype public headers
``FREETYPE_LIBRARIES``
  the library to link against
``FREETYPE_VERSION_STRING``
  the version of freetype found

.. versionadded:: 3.7
  Debug and Release variants are found separately.

Hints
^^^^^

The user may set the environment variable ``FREETYPE_DIR`` to the root
directory of a Freetype installation.
#]=======================================================================]

# Created by Eric Wing.
# Modifications by Alexander Neundorf.
# This file has been renamed to "FindFreetype.cmake" instead of the correct
# "FindFreeType.cmake" in order to be compatible with the one from KDE4, Alex.

# Ugh, FreeType seems to use some #include trickery which
# makes this harder than it should be. It looks like they
# put ft2build.h in a common/easier-to-find location which
# then contains a #include to a more specific header in a
# more specific location (#include <freetype/config/ftheader.h>).
# Then from there, they need to set a bunch of #define's
# so you can do something like:
# #include FT_FREETYPE_H
# Unfortunately, using CMake's mechanisms like include_directories()
# wants explicit full paths and this trickery doesn't work too well.
# I'm going to attempt to cut out the middleman and hope
# everything still works.

set(_Freetype_args)
if (Freetype_FIND_VERSION)
  list(APPEND _Freetype_args
    "${Freetype_FIND_VERSION}")
  if (Freetype_FIND_VERSION_EXACT)
    list(APPEND _Freetype_args
      EXACT)
  endif ()
endif ()
set(_Freetype_component_req)
set(_Freetype_component_opt)
foreach (_Freetype_component IN LISTS Freetype_FIND_COMPONENTS)
  if (Freetype_FIND_REQUIRE_${_Freetype_component})
    list(APPEND _Freetype_component_req
      "${_Freetype_component}")
  else ()
    list(APPEND _Freetype_component_opt
      "${_Freetype_component}")
  endif ()
endforeach ()
unset(_Freetype_component)
if (_Freetype_component_req)
  list(APPEND _Freetype_args
    COMPONENTS "${_Freetype_component_req}")
endif ()
unset(_Freetype_component_req)
if (_Freetype_component_opt)
  list(APPEND _Freetype_args
    OPTIONAL_COMPONENTS "${_Freetype_component_opt}")
endif ()
unset(_Freetype_component_opt)
# Always find with QUIET to avoid noise when it is not found.
find_package(freetype CONFIG QUIET ${_Freetype_args})
unset(_Freetype_args)
if (freetype_FOUND)
  if (NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype IMPORTED INTERFACE)
    set_target_properties(Freetype::Freetype PROPERTIES
      INTERFACE_LINK_LIBRARIES freetype)
  endif ()
  get_property(FREETYPE_INCLUDE_DIRS TARGET freetype PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  get_property(FREETYPE_LIBRARIES TARGET freetype PROPERTY INTERFACE_LINK_LIBRARIES)
  get_property(_Freetype_location TARGET freetype PROPERTY IMPORTED_IMPLIB)
  if (NOT _Freetype_location)
    get_property(_Freetype_location_release TARGET freetype PROPERTY IMPORTED_IMPLIB_RELEASE)
    if (NOT _Freetype_location_release)
      get_property(_Freetype_location_release TARGET freetype PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO)
    endif ()
    get_property(_Freetype_location_debug TARGET freetype PROPERTY IMPORTED_IMPLIB_DEBUG)
    if (_Freetype_location_release AND _Freetype_location_debug)
      set(_Freetype_location
        optimized "${_Freetype_location_release}"
        debug "${_Freetype_location_debug}")
    elseif (_Freetype_location_release)
      set(_Freetype_location "${_Freetype_location_release}")
    elseif (_Freetype_location_debug)
      set(_Freetype_location "${_Freetype_location_debug}")
    else ()
      get_property(_Freetype_location_release TARGET freetype PROPERTY LOCATION_RELEASE)
      if (NOT _Freetype_location_release)
        get_property(_Freetype_location_release TARGET freetype PROPERTY LOCATION_RELWITHDEBINFO)
      endif ()
      get_property(_Freetype_location_debug TARGET freetype PROPERTY LOCATION_DEBUG)
      if (_Freetype_location_release AND _Freetype_location_debug)
        set(_Freetype_location
          optimized "${_Freetype_location_release}"
          debug "${_Freetype_location_debug}")
      elseif (_Freetype_location_release)
        set(_Freetype_location "${_Freetype_location_release}")
      elseif (_Freetype_location_debug)
        set(_Freetype_location "${_Freetype_location_debug}")
      else ()
        get_property(_Freetype_location TARGET freetype PROPERTY LOCATION)
      endif ()
    endif ()
    unset(_Freetype_location_release)
    unset(_Freetype_location_debug)
  endif ()
  list(INSERT FREETYPE_LIBRARIES 0
    "${_Freetype_location}")
  unset(_Freetype_location)
  set(Freetype_FOUND 1)
  set(FREETYPE_FOUND 1)
  set(FREETYPE_VERSION_STRING "${freetype_VERSION}")
  foreach (_Freetype_component IN LISTS Freetype_FIND_COMPONENTS)
    set(Freetype_${_Freetype_component}_FOUND "${freetype_${_Freetype_component}_FOUND}")
  endforeach ()
  unset(_Freetype_component)

  include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
  find_package_handle_standard_args(Freetype
    HANDLE_COMPONENTS
    VERSION_VAR FREETYPE_VERSION_STRING
  )

  return ()
endif ()

set(FREETYPE_FIND_ARGS
  HINTS
    ENV FREETYPE_DIR
  PATHS
    ENV GTKMM_BASEPATH
    [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
)

find_path(
  FREETYPE_INCLUDE_DIR_ft2build
  ft2build.h
  ${FREETYPE_FIND_ARGS}
  PATH_SUFFIXES
    include/freetype2
    include
    freetype2
)

find_path(
  FREETYPE_INCLUDE_DIR_freetype2
  NAMES
    freetype/config/ftheader.h
    config/ftheader.h
  ${FREETYPE_FIND_ARGS}
  PATH_SUFFIXES
    include/freetype2
    include
    freetype2
)

if(NOT FREETYPE_LIBRARY)
  find_library(FREETYPE_LIBRARY_RELEASE
    NAMES
      freetype
      libfreetype
      freetype219
    ${FREETYPE_FIND_ARGS}
    PATH_SUFFIXES
      lib
  )
  find_library(FREETYPE_LIBRARY_DEBUG
    NAMES
      freetyped
      libfreetyped
      freetype219d
    ${FREETYPE_FIND_ARGS}
    PATH_SUFFIXES
      lib
  )
  include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
  select_library_configurations(FREETYPE)
else()
  # on Windows, ensure paths are in canonical format (forward slahes):
  file(TO_CMAKE_PATH "${FREETYPE_LIBRARY}" FREETYPE_LIBRARY)
endif()

unset(FREETYPE_FIND_ARGS)

# set the user variables
if(FREETYPE_INCLUDE_DIR_ft2build AND FREETYPE_INCLUDE_DIR_freetype2)
  set(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build};${FREETYPE_INCLUDE_DIR_freetype2}")
  list(REMOVE_DUPLICATES FREETYPE_INCLUDE_DIRS)
endif()
set(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")

if(EXISTS "${FREETYPE_INCLUDE_DIR_freetype2}/freetype/freetype.h")
  set(FREETYPE_H "${FREETYPE_INCLUDE_DIR_freetype2}/freetype/freetype.h")
elseif(EXISTS "${FREETYPE_INCLUDE_DIR_freetype2}/freetype.h")
  set(FREETYPE_H "${FREETYPE_INCLUDE_DIR_freetype2}/freetype.h")
endif()

if(FREETYPE_INCLUDE_DIR_freetype2 AND FREETYPE_H)
  file(STRINGS "${FREETYPE_H}" freetype_version_str
       REGEX "^#[\t ]*define[\t ]+FREETYPE_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

  unset(FREETYPE_VERSION_STRING)
  foreach(VPART MAJOR MINOR PATCH)
    foreach(VLINE ${freetype_version_str})
      if(VLINE MATCHES "^#[\t ]*define[\t ]+FREETYPE_${VPART}[\t ]+([0-9]+)$")
        set(FREETYPE_VERSION_PART "${CMAKE_MATCH_1}")
        if(FREETYPE_VERSION_STRING)
          string(APPEND FREETYPE_VERSION_STRING ".${FREETYPE_VERSION_PART}")
        else()
          set(FREETYPE_VERSION_STRING "${FREETYPE_VERSION_PART}")
        endif()
        unset(FREETYPE_VERSION_PART)
      endif()
    endforeach()
  endforeach()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

find_package_handle_standard_args(
  Freetype
  REQUIRED_VARS
    FREETYPE_LIBRARY
    FREETYPE_INCLUDE_DIRS
  VERSION_VAR
    FREETYPE_VERSION_STRING
)

mark_as_advanced(
  FREETYPE_INCLUDE_DIR_freetype2
  FREETYPE_INCLUDE_DIR_ft2build
)

if(Freetype_FOUND)
  if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype UNKNOWN IMPORTED)
    set_target_properties(Freetype::Freetype PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIRS}")

    if(FREETYPE_LIBRARY_RELEASE)
      set_property(TARGET Freetype::Freetype APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION_RELEASE "${FREETYPE_LIBRARY_RELEASE}")
    endif()

    if(FREETYPE_LIBRARY_DEBUG)
      set_property(TARGET Freetype::Freetype APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
        IMPORTED_LOCATION_DEBUG "${FREETYPE_LIBRARY_DEBUG}")
    endif()

    if(NOT FREETYPE_LIBRARY_RELEASE AND NOT FREETYPE_LIBRARY_DEBUG)
      set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${FREETYPE_LIBRARY}")
    endif()
  endif()
endif()
