# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindGLUT
--------

Find OpenGL Utility Toolkit (GLUT) library and include files.

IMPORTED Targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.1

This module defines the :prop_tgt:`IMPORTED` targets:

``GLUT::GLUT``
 Defined if the system has GLUT.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``GLUT_FOUND``
  True if ``glut`` was found.

``GLUT_INCLUDE_DIRS``
  .. versionadded:: 3.23

  Where to find GL/glut.h, etc.

``GLUT_LIBRARIES``
  List of libraries for using ``glut``.

Cache Variables
^^^^^^^^^^^^^^^

This module may set the following variables depending on platform.
These variables may optionally be set to help this module find the
correct files, but clients should not use these as results:

``GLUT_INCLUDE_DIR``
  The full path to the directory containing ``GL/glut.h``,
  not including ``GL/``.

``GLUT_glut_LIBRARY``
  The full path to the glut library.

``GLUT_Xmu_LIBRARY``
  The full path to the Xmu library.

``GLUT_Xi_LIBRARY``
  The full path to the Xi Library.

Obsolete Variables
^^^^^^^^^^^^^^^^^^

The following variables may also be provided, for backwards compatibility:

``GLUT_INCLUDE_DIR``
  This is one of above `Cache Variables`_, but prior to CMake 3.23 was
  also a result variable.  Prefer to use ``GLUT_INCLUDE_DIRS`` instead
  in CMake 3.23 and above.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

function(_add_glut_target_simple)
  if(TARGET GLUT::GLUT)
    return()
  endif()
  add_library(GLUT::GLUT INTERFACE IMPORTED)
  if(GLUT_INCLUDE_DIRS)
    target_include_directories(GLUT::GLUT SYSTEM
      INTERFACE "${GLUT_INCLUDE_DIRS}")
  endif()
  if(GLUT_LIBRARIES)
    target_link_libraries(GLUT::GLUT INTERFACE ${GLUT_LIBRARIES})
  endif()
  if(GLUT_LIBRARY_DIRS)
    target_link_directories(GLUT::GLUT INTERFACE ${GLUT_LIBRARY_DIRS})
  endif()
  if(GLUT_LDFLAGS)
    target_link_options(GLUT::GLUT INTERFACE ${GLUT_LDFLAGS})
  endif()
  if(GLUT_CFLAGS)
    separate_arguments(GLUT_CFLAGS_SPLIT UNIX_COMMAND "${GLUT_CFLAGS}")
    target_compile_options(GLUT::GLUT INTERFACE ${GLUT_CFLAGS_SPLIT})
  endif()

  set_property(TARGET GLUT::GLUT APPEND PROPERTY
    IMPORTED_LOCATION "${GLUT_glut_LIBRARY}")
endfunction()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(GLUT glut)
  if(GLUT_FOUND)
    # GLUT_INCLUDE_DIRS is now the official result variable, but
    # older versions of CMake only provided GLUT_INCLUDE_DIR.
    set(GLUT_INCLUDE_DIR "${GLUT_INCLUDE_DIRS}")
    _add_glut_target_simple()
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLUT REQUIRED_VARS GLUT_FOUND)
    return()
  endif()
endif()

if(WIN32)
  find_path( GLUT_INCLUDE_DIR NAMES GL/glut.h
    PATHS  ${GLUT_ROOT_PATH}/include )
  mark_as_advanced(GLUT_INCLUDE_DIR)
  find_library( GLUT_glut_LIBRARY_RELEASE NAMES glut glut32 freeglut
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${GLUT_ROOT_PATH}/Release
    )
  find_library( GLUT_glut_LIBRARY_DEBUG NAMES freeglutd
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${GLUT_ROOT_PATH}/Debug
    )
  mark_as_advanced(GLUT_glut_LIBRARY_RELEASE GLUT_glut_LIBRARY_DEBUG)
  select_library_configurations(GLUT_glut)
elseif(APPLE)
  find_path(GLUT_INCLUDE_DIR glut.h ${OPENGL_LIBRARY_DIR})
  mark_as_advanced(GLUT_INCLUDE_DIR)
  find_library(GLUT_glut_LIBRARY GLUT DOC "GLUT library for OSX")
  find_library(GLUT_cocoa_LIBRARY Cocoa DOC "Cocoa framework for OSX")
  mark_as_advanced(GLUT_glut_LIBRARY GLUT_cocoa_LIBRARY)

  if(GLUT_cocoa_LIBRARY AND NOT TARGET GLUT::Cocoa)
    add_library(GLUT::Cocoa UNKNOWN IMPORTED)
    # Cocoa should always be a Framework, but we check to make sure.
    if(GLUT_cocoa_LIBRARY MATCHES "/([^/]+)\\.framework$")
      set(_glut_cocoa "${GLUT_cocoa_LIBRARY}/${CMAKE_MATCH_1}")
      if(EXISTS "${_glut_cocoa}.tbd")
        string(APPEND _glut_cocoa ".tbd")
      endif()
      set_target_properties(GLUT::Cocoa PROPERTIES
        IMPORTED_LOCATION "${_glut_cocoa}")
    else()
      set_target_properties(GLUT::Cocoa PROPERTIES
        IMPORTED_LOCATION "${GLUT_cocoa_LIBRARY}")
    endif()
  endif()
else()
  if(BEOS)
    set(_GLUT_INC_DIR /boot/develop/headers/os/opengl)
    set(_GLUT_glut_LIB_DIR /boot/develop/lib/x86)
  else()
    find_library( GLUT_Xi_LIBRARY Xi
      /usr/openwin/lib
      )
    mark_as_advanced(GLUT_Xi_LIBRARY)

    find_library( GLUT_Xmu_LIBRARY Xmu
      /usr/openwin/lib
      )
    mark_as_advanced(GLUT_Xmu_LIBRARY)

    if(GLUT_Xi_LIBRARY AND NOT TARGET GLUT::Xi)
      add_library(GLUT::Xi UNKNOWN IMPORTED)
      set_target_properties(GLUT::Xi PROPERTIES
        IMPORTED_LOCATION "${GLUT_Xi_LIBRARY}")
    endif()

    if(GLUT_Xmu_LIBRARY AND NOT TARGET GLUT::Xmu)
      add_library(GLUT::Xmu UNKNOWN IMPORTED)
      set_target_properties(GLUT::Xmu PROPERTIES
        IMPORTED_LOCATION "${GLUT_Xmu_LIBRARY}")
    endif()

  endif ()

  find_path( GLUT_INCLUDE_DIR GL/glut.h
    /usr/include/GL
    /usr/openwin/share/include
    /usr/openwin/include
    /opt/graphics/OpenGL/include
    /opt/graphics/OpenGL/contrib/libglut
    ${_GLUT_INC_DIR}
    )
  mark_as_advanced(GLUT_INCLUDE_DIR)

  find_library( GLUT_glut_LIBRARY glut
    /usr/openwin/lib
    ${_GLUT_glut_LIB_DIR}
    )
  mark_as_advanced(GLUT_glut_LIBRARY)

  unset(_GLUT_INC_DIR)
  unset(_GLUT_glut_LIB_DIR)
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLUT REQUIRED_VARS GLUT_glut_LIBRARY GLUT_INCLUDE_DIR)

if (GLUT_FOUND)
  # Is -lXi and -lXmu required on all platforms that have it?
  # If not, we need some way to figure out what platform we are on.
  set( GLUT_LIBRARIES
    ${GLUT_glut_LIBRARY}
    )
  set(GLUT_INCLUDE_DIRS
    ${GLUT_INCLUDE_DIR}
    )
  foreach(v GLUT_Xmu_LIBRARY GLUT_Xi_LIBRARY GLUT_cocoa_LIBRARY)
    if(${v})
      list(APPEND GLUT_LIBRARIES ${${v}})
    endif()
  endforeach()

  if(NOT TARGET GLUT::GLUT)
    add_library(GLUT::GLUT UNKNOWN IMPORTED)
    set_target_properties(GLUT::GLUT PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GLUT_INCLUDE_DIRS}")
    if(GLUT_glut_LIBRARY MATCHES "/([^/]+)\\.framework$")
      set(_glut_glut "${GLUT_glut_LIBRARY}/${CMAKE_MATCH_1}")
      if(EXISTS "${_glut_glut}.tbd")
        string(APPEND _glut_glut ".tbd")
      endif()
      set_target_properties(GLUT::GLUT PROPERTIES
        IMPORTED_LOCATION "${_glut_glut}")
    else()
      if(GLUT_glut_LIBRARY_RELEASE)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(GLUT::GLUT PROPERTIES
          IMPORTED_LOCATION_RELEASE "${GLUT_glut_LIBRARY_RELEASE}")
      endif()

      if(GLUT_glut_LIBRARY_DEBUG)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(GLUT::GLUT PROPERTIES
          IMPORTED_LOCATION_DEBUG "${GLUT_glut_LIBRARY_DEBUG}")
      endif()

      if(NOT GLUT_glut_LIBRARY_RELEASE AND NOT GLUT_glut_LIBRARY_DEBUG)
        set_property(TARGET GLUT::GLUT APPEND PROPERTY
          IMPORTED_LOCATION "${GLUT_glut_LIBRARY}")
      endif()
    endif()

    if(TARGET GLUT::Xmu)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Xmu)
    endif()

    if(TARGET GLUT::Xi)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Xi)
    endif()

    if(TARGET GLUT::Cocoa)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Cocoa)
    endif()
  endif()

  #The following deprecated settings are for backwards compatibility with CMake1.4
  set (GLUT_LIBRARY ${GLUT_LIBRARIES})
  set (GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIRS})
endif()
