# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindFontconfig
--------------

Find Fontconfig headers and library.

Imported Targets
^^^^^^^^^^^^^^^^

``Fontconfig::Fontconfig``
  The Fontconfig library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``FONTCONFIG_FOUND``
  true if (the requested version of) Fontconfig is available.
``FONTCONFIG_VERSION``
  the version of Fontconfig.
``FONTCONFIG_LIBRARIES``
  the libraries to link against to use Fontconfig.
``FONTCONFIG_INCLUDE_DIRS``
  where to find the Fontconfig headers.
``FONTCONFIG_COMPILE_OPTIONS``
  this should be passed to target_compile_options(), if the
  target is not used for linking

#]=======================================================================]


# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_FONTCONFIG QUIET fontconfig)
set(FONTCONFIG_COMPILE_OPTIONS ${PKG_FONTCONFIG_CFLAGS_OTHER})
set(FONTCONFIG_VERSION ${PKG_FONTCONFIG_VERSION})

find_path( FONTCONFIG_INCLUDE_DIR
  NAMES
    fontconfig/fontconfig.h
  HINTS
    ${PKG_FONTCONFIG_INCLUDE_DIRS}
    /usr/X11/include
)

find_library( FONTCONFIG_LIBRARY
  NAMES
    fontconfig
  PATHS
    ${PKG_FONTCONFIG_LIBRARY_DIRS}
)

if (FONTCONFIG_INCLUDE_DIR AND NOT FONTCONFIG_VERSION)
  file(STRINGS ${FONTCONFIG_INCLUDE_DIR}/fontconfig/fontconfig.h _contents REGEX "^#define[ \t]+FC_[A-Z]+[ \t]+[0-9]+$")
  unset(FONTCONFIG_VERSION)
  foreach(VPART MAJOR MINOR REVISION)
    foreach(VLINE ${_contents})
      if(VLINE MATCHES "^#define[\t ]+FC_${VPART}[\t ]+([0-9]+)$")
        set(FONTCONFIG_VERSION_PART "${CMAKE_MATCH_1}")
        if(FONTCONFIG_VERSION)
          string(APPEND FONTCONFIG_VERSION ".${FONTCONFIG_VERSION_PART}")
        else()
          set(FONTCONFIG_VERSION "${FONTCONFIG_VERSION_PART}")
        endif()
      endif()
    endforeach()
  endforeach()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fontconfig
  FOUND_VAR
    FONTCONFIG_FOUND
  REQUIRED_VARS
    FONTCONFIG_LIBRARY
    FONTCONFIG_INCLUDE_DIR
  VERSION_VAR
    FONTCONFIG_VERSION
)


if(FONTCONFIG_FOUND AND NOT TARGET Fontconfig::Fontconfig)
  add_library(Fontconfig::Fontconfig UNKNOWN IMPORTED)
  set_target_properties(Fontconfig::Fontconfig PROPERTIES
    IMPORTED_LOCATION "${FONTCONFIG_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${FONTCONFIG_COMPILE_OPTIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${FONTCONFIG_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(FONTCONFIG_LIBRARY FONTCONFIG_INCLUDE_DIR)

if(FONTCONFIG_FOUND)
  set(FONTCONFIG_LIBRARIES ${FONTCONFIG_LIBRARY})
  set(FONTCONFIG_INCLUDE_DIRS ${FONTCONFIG_INCLUDE_DIR})
endif()
