# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindTIFF
--------

Find the TIFF library (``libtiff``, https://libtiff.gitlab.io/libtiff/).

Optional COMPONENTS
^^^^^^^^^^^^^^^^^^^

This module supports the optional component `CXX`, for use with the COMPONENTS
argument of the :command:`find_package` command. This component has an associated
imported target, as described below.

Imported targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.5

This module defines the following :prop_tgt:`IMPORTED` targets:

``TIFF::TIFF``
  The TIFF library, if found.

``TIFF::CXX``
  .. versionadded:: 3.19

  The C++ wrapper libtiffxx, if requested by the `COMPONENTS CXX` option,
  if the compiler is not MSVC (which includes the C++ wrapper in libtiff),
  and if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``TIFF_FOUND``
  true if the TIFF headers and libraries were found
``TIFF_INCLUDE_DIR``
  the directory containing the TIFF headers
``TIFF_INCLUDE_DIRS``
  the directory containing the TIFF headers
``TIFF_LIBRARIES``
  TIFF libraries to be linked

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``TIFF_INCLUDE_DIR``
  the directory containing the TIFF headers
``TIFF_LIBRARY_RELEASE``
  the path to the TIFF library for release configurations
``TIFF_LIBRARY_DEBUG``
  the path to the TIFF library for debug configurations
``TIFFXX_LIBRARY_RELEASE``
  the path to the TIFFXX library for release configurations
``TIFFXX_LIBRARY_DEBUG``
  the path to the TIFFXX library for debug configurations

.. versionadded:: 3.4
  Debug and Release variants are found separately.
#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0057 NEW) # if IN_LIST

set(_TIFF_args)
if (TIFF_FIND_VERSION)
  list(APPEND _TIFF_args
    "${TIFF_FIND_VERSION}")
  if (TIFF_FIND_VERSION_EXACT)
    list(APPEND _TIFF_args
      EXACT)
  endif ()
endif ()
set(_TIFF_component_req)
set(_TIFF_component_opt)
foreach (_TIFF_component IN LISTS TIFF_FIND_COMPONENTS)
  if (TIFF_FIND_REQUIRE_${_TIFF_component})
    list(APPEND _TIFF_component_req
      "${_TIFF_component}")
  else ()
    list(APPEND _TIFF_component_opt
      "${_TIFF_component}")
  endif ()
endforeach ()
unset(_TIFF_component)
if (_TIFF_component_req)
  list(APPEND _TIFF_args
    COMPONENTS "${_TIFF_component_req}")
endif ()
unset(_TIFF_component_req)
if (_TIFF_component_opt)
  list(APPEND _TIFF_args
    OPTIONAL_COMPONENTS "${_TIFF_component_opt}")
endif ()
unset(_TIFF_component_opt)
# Always find with QUIET to avoid noise when it is not found.
find_package(tiff CONFIG QUIET ${_TIFF_args})
unset(_TIFF_args)
if (tiff_FOUND)
  if (NOT TARGET TIFF::TIFF)
    add_library(TIFF::TIFF IMPORTED INTERFACE)
    set_target_properties(TIFF::TIFF PROPERTIES
      INTERFACE_LINK_LIBRARIES TIFF::tiff)
  endif ()
  get_property(TIFF_INCLUDE_DIRS TARGET TIFF::tiff PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  get_property(TIFF_LIBRARIES TARGET TIFF::tiff PROPERTY INTERFACE_LINK_LIBRARIES)
  get_property(_TIFF_location TARGET TIFF::tiff PROPERTY IMPORTED_IMPLIB)
  if (NOT _TIFF_location)
    get_property(_TIFF_location_release TARGET TIFF::tiff PROPERTY IMPORTED_IMPLIB_RELEASE)
    if (NOT _TIFF_location_release)
      get_property(_TIFF_location_release TARGET TIFF::tiff PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO)
    endif ()
    get_property(_TIFF_location_debug TARGET TIFF::tiff PROPERTY IMPORTED_IMPLIB_DEBUG)
    if (_TIFF_location_release AND _TIFF_location_debug)
      set(_TIFF_location
        optimized "${_TIFF_location_release}"
        debug "${_TIFF_location_debug}")
    elseif (_TIFF_location_release)
      set(_TIFF_location "${_TIFF_location_release}")
    elseif (_TIFF_location_debug)
      set(_TIFF_location "${_TIFF_location_debug}")
    else ()
      get_property(_TIFF_location_release TARGET TIFF::tiff PROPERTY LOCATION_RELEASE)
      if (NOT _TIFF_location_release)
        get_property(_TIFF_location_release TARGET TIFF::tiff PROPERTY LOCATION_RELWITHDEBINFO)
      endif ()
      get_property(_TIFF_location_debug TARGET TIFF::tiff PROPERTY LOCATION_DEBUG)
      if (_TIFF_location_release AND _TIFF_location_debug)
        set(_TIFF_location
          optimized "${_TIFF_location_release}"
          debug "${_TIFF_location_debug}")
      elseif (_TIFF_location_release)
        set(_TIFF_location "${_TIFF_location_release}")
      elseif (_TIFF_location_debug)
        set(_TIFF_location "${_TIFF_location_debug}")
      else ()
        get_property(_TIFF_location TARGET TIFF::tiff PROPERTY LOCATION)
      endif ()
    endif ()
    unset(_TIFF_location_release)
    unset(_TIFF_location_debug)
  endif ()
  list(INSERT TIFF_LIBRARIES 0
    "${_TIFF_location}")
  unset(_TIFF_location)
  set(TIFF_FOUND 1)
  if("CXX" IN_LIST TIFF_FIND_COMPONENTS)
    if (TARGET TIFF::CXX)
      get_property(_TIFF_CXX_location TARGET TIFF::CXX PROPERTY IMPORTED_IMPLIB)
      if (NOT _TIFF_CXX_location)
        get_property(_TIFF_CXX_location_release TARGET TIFF::CXX PROPERTY IMPORTED_IMPLIB_RELEASE)
        if (NOT _TIFF_CXX_location_release)
          get_property(_TIFF_CXX_location_release TARGET TIFF::CXX PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO)
        endif ()
        get_property(_TIFF_CXX_location_debug TARGET TIFF::CXX PROPERTY IMPORTED_IMPLIB_DEBUG)
        if (_TIFF_CXX_location_release AND _TIFF_CXX_location_debug)
          set(_TIFF_CXX_location
            optimized "${_TIFF_CXX_location_release}"
            debug "${_TIFF_CXX_location_debug}")
        elseif (_TIFF_CXX_location_release)
          set(_TIFF_CXX_location "${_TIFF_CXX_location_release}")
        elseif (_TIFF_CXX_location_debug)
          set(_TIFF_CXX_location "${_TIFF_CXX_location_debug}")
        else ()
          get_property(_TIFF_CXX_location_release TARGET TIFF::CXX PROPERTY LOCATION_RELEASE)
          if (NOT _TIFF_CXX_location_release)
            get_property(_TIFF_CXX_location_release TARGET TIFF::CXX PROPERTY LOCATION_RELWITHDEBINFO)
          endif ()
          get_property(_TIFF_CXX_location_debug TARGET TIFF::CXX PROPERTY LOCATION_DEBUG)
          if (_TIFF_CXX_location_release AND _TIFF_CXX_location_debug)
            set(_TIFF_CXX_location
              optimized "${_TIFF_CXX_location_release}"
              debug "${_TIFF_CXX_location_debug}")
          elseif (_TIFF_CXX_location_release)
            set(_TIFF_CXX_location "${_TIFF_CXX_location_release}")
          elseif (_TIFF_CXX_location_debug)
            set(_TIFF_CXX_location "${_TIFF_CXX_location_debug}")
          else ()
            get_property(_TIFF_CXX_location TARGET TIFF::CXX PROPERTY LOCATION)
          endif ()
        endif ()
        unset(_TIFF_CXX_location_release)
        unset(_TIFF_CXX_location_debug)
      endif ()
      list(INSERT TIFF_LIBRARIES 0
        "${_TIFF_CXX_location}")
      unset(_TIFF_CXX_location)
      set(TIFF_CXX_FOUND 1)
    else ()
      set(TIFF_CXX_FOUND 0)
      if (TIFF_FIND_REQUIRED_CXX)
        set(TIFF_FOUND 0)
        list(APPEND TIFF_NOT_FOUND_REASON
          "No C++ bindings target found")
      endif ()
    endif ()
  endif ()
  set(TIFF_VERSION_STRING "${tiff_VERSION}")
  foreach (_TIFF_component IN LISTS TIFF_FIND_COMPONENTS)
    set(TIFF_${_TIFF_component}_FOUND "${tiff_${_TIFF_component}_FOUND}")
  endforeach ()
  unset(_TIFF_component)

  include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
  find_package_handle_standard_args(TIFF
                                    HANDLE_COMPONENTS
                                    VERSION_VAR TIFF_VERSION_STRING)

  cmake_policy(POP)
  return ()
endif ()

find_path(TIFF_INCLUDE_DIR tiff.h)

set(TIFF_NAMES ${TIFF_NAMES} tiff libtiff tiff3 libtiff3)
foreach(name ${TIFF_NAMES})
  list(APPEND TIFF_NAMES_DEBUG "${name}d")
endforeach()

if(NOT TIFF_LIBRARY)
  find_library(TIFF_LIBRARY_RELEASE NAMES ${TIFF_NAMES})
  find_library(TIFF_LIBRARY_DEBUG NAMES ${TIFF_NAMES_DEBUG})
  include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
  select_library_configurations(TIFF)
  mark_as_advanced(TIFF_LIBRARY_RELEASE TIFF_LIBRARY_DEBUG)
endif()

if(TIFF_INCLUDE_DIR AND EXISTS "${TIFF_INCLUDE_DIR}/tiffvers.h")
    file(STRINGS "${TIFF_INCLUDE_DIR}/tiffvers.h" tiff_version_str
         REGEX "^#define[\t ]+TIFFLIB_VERSION_STR[\t ]+\"LIBTIFF, Version .*")

    string(REGEX REPLACE "^#define[\t ]+TIFFLIB_VERSION_STR[\t ]+\"LIBTIFF, Version +([^ \\n]*).*"
           "\\1" TIFF_VERSION_STRING "${tiff_version_str}")
    unset(tiff_version_str)
endif()

foreach(_comp IN LISTS TIFF_FIND_COMPONENTS)
  if(_comp STREQUAL "CXX")
    if(MSVC)
      # C++ bindings are built into the main tiff library.
      set(TIFF_CXX_FOUND 1)
    else()
      foreach(name ${TIFF_NAMES})
        list(APPEND TIFFXX_NAMES "${name}xx")
        list(APPEND TIFFXX_NAMES_DEBUG "${name}xxd")
      endforeach()
      find_library(TIFFXX_LIBRARY_RELEASE NAMES ${TIFFXX_NAMES})
      find_library(TIFFXX_LIBRARY_DEBUG NAMES ${TIFFXX_NAMES_DEBUG})
      include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
      select_library_configurations(TIFFXX)
      mark_as_advanced(TIFFXX_LIBRARY_RELEASE TIFFXX_LIBRARY_DEBUG)
      unset(TIFFXX_NAMES)
      unset(TIFFXX_NAMES_DEBUG)
      if(TIFFXX_LIBRARY)
        set(TIFF_CXX_FOUND 1)
      endif()
    endif()
  endif()
endforeach()
unset(_comp)

unset(TIFF_NAMES)
unset(TIFF_NAMES_DEBUG)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TIFF
                                  HANDLE_COMPONENTS
                                  REQUIRED_VARS TIFF_LIBRARY TIFF_INCLUDE_DIR
                                  VERSION_VAR TIFF_VERSION_STRING)

if(TIFF_FOUND)
  set(TIFF_LIBRARIES ${TIFF_LIBRARY})
  if("CXX" IN_LIST TIFF_FIND_COMPONENTS AND NOT MSVC)
    list(APPEND TIFF_LIBRARIES ${TIFFXX_LIBRARY})
  endif()

  set(TIFF_INCLUDE_DIRS "${TIFF_INCLUDE_DIR}")

  if(NOT TARGET TIFF::TIFF)
    add_library(TIFF::TIFF UNKNOWN IMPORTED)
    if(TIFF_INCLUDE_DIRS)
      set_target_properties(TIFF::TIFF PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${TIFF_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${TIFF_LIBRARY}")
      set_target_properties(TIFF::TIFF PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${TIFF_LIBRARY}")
    endif()
    if(EXISTS "${TIFF_LIBRARY_RELEASE}")
      set_property(TARGET TIFF::TIFF APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(TIFF::TIFF PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION_RELEASE "${TIFF_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${TIFF_LIBRARY_DEBUG}")
      set_property(TARGET TIFF::TIFF APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(TIFF::TIFF PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
        IMPORTED_LOCATION_DEBUG "${TIFF_LIBRARY_DEBUG}")
    endif()
  endif()

  if(NOT TARGET TIFF::CXX)
    if(MSVC)
      add_library(TIFF::CXX INTERFACE IMPORTED)
      set_property(TARGET TIFF::CXX PROPERTY INTERFACE_LINK_LIBRARIES TIFF::TIFF)
    else()
      add_library(TIFF::CXX UNKNOWN IMPORTED)
      set_property(TARGET TIFF::CXX PROPERTY INTERFACE_LINK_LIBRARIES TIFF::TIFF)
      if(TIFF_INCLUDE_DIRS)
        set_target_properties(TIFF::CXX PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${TIFF_INCLUDE_DIRS}")
      endif()
      if(EXISTS "${TIFFXX_LIBRARY}")
        set_target_properties(TIFF::CXX PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
          IMPORTED_LOCATION "${TIFFXX_LIBRARY}")
      endif()
      if(EXISTS "${TIFFXX_LIBRARY_RELEASE}")
        set_property(TARGET TIFF::CXX APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(TIFF::CXX PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
          IMPORTED_LOCATION_RELEASE "${TIFFXX_LIBRARY_RELEASE}")
      endif()
      if(EXISTS "${TIFFXX_LIBRARY_DEBUG}")
        set_property(TARGET TIFF::CXX APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(TIFF::CXX PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
          IMPORTED_LOCATION_DEBUG "${TIFFXX_LIBRARY_DEBUG}")
      endif()
    endif()
  endif()

endif()

mark_as_advanced(TIFF_INCLUDE_DIR)
cmake_policy(POP)
