# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindJasper
----------

Find the Jasper JPEG2000 library.

Imported Targets
^^^^^^^^^^^^^^^^

``Jasper::Jasper``
  The jasper library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``JASPER_FOUND``
  system has Jasper
``JASPER_INCLUDE_DIRS``
  .. versionadded:: 3.22

  the Jasper include directory
``JASPER_LIBRARIES``
  the libraries needed to use Jasper
``JASPER_VERSION_STRING``
  the version of Jasper found

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``JASPER_INCLUDE_DIR``
  where to find jasper/jasper.h, etc.
``JASPER_LIBRARY_RELEASE``
  where to find the Jasper library (optimized).
``JASPER_LIBARRY_DEBUG``
  where to find the Jasper library (debug).
#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

find_path(JASPER_INCLUDE_DIR jasper/jasper.h)
mark_as_advanced(JASPER_INCLUDE_DIR)

if(NOT JASPER_LIBRARIES)
  find_package(JPEG)
  find_library(JASPER_LIBRARY_RELEASE NAMES jasper libjasper)
  find_library(JASPER_LIBRARY_DEBUG NAMES jasperd)
  include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
  select_library_configurations(JASPER)
endif()

if(JASPER_INCLUDE_DIR AND EXISTS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h")
  file(STRINGS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h" jasper_version_str REGEX "^#define[\t ]+JAS_VERSION[\t ]+\".*\".*")
  string(REGEX REPLACE "^#define[\t ]+JAS_VERSION[\t ]+\"([^\"]+)\".*" "\\1" JASPER_VERSION_STRING "${jasper_version_str}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Jasper
                                  REQUIRED_VARS JASPER_LIBRARIES JASPER_INCLUDE_DIR JPEG_LIBRARIES
                                  VERSION_VAR JASPER_VERSION_STRING)

if(JASPER_FOUND)
  set(JASPER_LIBRARIES ${JASPER_LIBRARIES} ${JPEG_LIBRARIES})
  set(JASPER_INCLUDE_DIRS ${JASPER_INCLUDE_DIR})
  if(NOT TARGET Jasper::Jasper)
    add_library(Jasper::Jasper UNKNOWN IMPORTED)
    if(JASPER_INCLUDE_DIRS)
      set_target_properties(Jasper::Jasper PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${JASPER_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${JASPER_LIBRARY_RELEASE}")
      set_property(TARGET Jasper::Jasper APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(Jasper::Jasper PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION "${JASPER_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${JASPER_LIBRARY_DEBUG}")
      set_property(TARGET Jasper::Jasper APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(Jasper::Jasper PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
        IMPORTED_LOCATION "${JASPER_LIBRARY_DEBUG}")
    endif()
  endif()
endif()

cmake_policy(POP)
