# - Try to find OpenGL
# Once done this will define
#
#  OPENGL_FOUND        - system has OpenGL
#  OPENGL_XMESA_FOUND  - system has XMESA
#  OPENGL_GLU_FOUND    - system has GLU
#  OPENGL_INCLUDE_DIR  - the GL include directory
#  OPENGL_LIBRARIES    - Link these to use OpenGL and GLU
#
# If you want to use just GL you can use these values
#  OPENGL_gl_LIBRARY   - Path to OpenGL Library
#  OPENGL_glu_LIBRARY  - Path to GLU Library
#
# On OSX default to using the framework version of opengl
# People will have to change the cache values of OPENGL_glu_LIBRARY
# and OPENGL_gl_LIBRARY to use OpenGL with X11 on OSX

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (CYGWIN)

  find_path(OPENGL_INCLUDE_DIR GL/gl.h )

  find_library(OPENGL_gl_LIBRARY opengl32 )

  find_library(OPENGL_glu_LIBRARY glu32 )

elseif (WIN32)

  if(BORLAND)
    set (OPENGL_gl_LIBRARY import32 CACHE STRING "OpenGL library for win32")
    set (OPENGL_glu_LIBRARY import32 CACHE STRING "GLU library for win32")
  else()
    set (OPENGL_gl_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
    set (OPENGL_glu_LIBRARY glu32 CACHE STRING "GLU library for win32")
  endif()

elseif (APPLE)

  find_library(OPENGL_gl_LIBRARY OpenGL DOC "OpenGL lib for OSX")
  find_library(OPENGL_glu_LIBRARY AGL DOC "AGL lib for OSX")
  find_path(OPENGL_INCLUDE_DIR OpenGL/gl.h DOC "Include for OpenGL on OSX")

else()
  if (CMAKE_SYSTEM_NAME MATCHES "HP-UX")
    # Handle HP-UX cases where we only want to find OpenGL in either hpux64
    # or hpux32 depending on if we're doing a 64 bit build.
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      set(_OPENGL_LIB_PATH
        /opt/graphics/OpenGL/lib/hpux32/)
    else()
      set(_OPENGL_LIB_PATH
        /opt/graphics/OpenGL/lib/hpux64/
        /opt/graphics/OpenGL/lib/pa20_64)
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL Haiku)
    set(_OPENGL_LIB_PATH
      /boot/develop/lib/x86)
    set(_OPENGL_INCLUDE_PATH
      /boot/develop/headers/os/opengl)
  endif()

  # The first line below is to make sure that the proper headers
  # are used on a Linux machine with the NVidia drivers installed.
  # They replace Mesa with NVidia's own library but normally do not
  # install headers and that causes the linking to
  # fail since the compiler finds the Mesa headers but NVidia's library.
  # Make sure the NVIDIA directory comes BEFORE the others.
  #  - Atanas Georgiev <atanas@cs.columbia.edu>

  find_path(OPENGL_INCLUDE_DIR GL/gl.h
    /usr/share/doc/NVIDIA_GLX-1.0/include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include /usr/X11R6/include
    ${_OPENGL_INCLUDE_PATH}
  )

  find_path(OPENGL_xmesa_INCLUDE_DIR GL/xmesa.h
    /usr/share/doc/NVIDIA_GLX-1.0/include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include /usr/X11R6/include
  )

  find_library(OPENGL_gl_LIBRARY
    NAMES GL MesaGL
    PATHS /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/shlib /usr/X11R6/lib
          ${_OPENGL_LIB_PATH}
  )

  unset(_OPENGL_INCLUDE_PATH)
  unset(_OPENGL_LIB_PATH)

  # On Unix OpenGL most certainly always requires X11.
  # Feel free to tighten up these conditions if you don't
  # think this is always true.

  if (OPENGL_gl_LIBRARY)
    if(NOT X11_FOUND)
      include(${CMAKE_CURRENT_LIST_DIR}/FindX11.cmake)
    endif()
    if (X11_FOUND)
      set (OPENGL_LIBRARIES ${X11_LIBRARIES})
    endif ()
  endif ()

  find_library(OPENGL_glu_LIBRARY
    NAMES GLU MesaGLU
    PATHS ${OPENGL_gl_LIBRARY}
          /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/shlib /usr/X11R6/lib
  )

endif ()

if(OPENGL_gl_LIBRARY)

    if(OPENGL_xmesa_INCLUDE_DIR)
      set( OPENGL_XMESA_FOUND "YES" )
    else()
      set( OPENGL_XMESA_FOUND "NO" )
    endif()

    set( OPENGL_LIBRARIES  ${OPENGL_gl_LIBRARY} ${OPENGL_LIBRARIES})
    if(OPENGL_glu_LIBRARY)
      set( OPENGL_GLU_FOUND "YES" )
      set( OPENGL_LIBRARIES ${OPENGL_glu_LIBRARY} ${OPENGL_LIBRARIES} )
    else()
      set( OPENGL_GLU_FOUND "NO" )
    endif()

    # This deprecated setting is for backward compatibility with CMake1.4
    set (OPENGL_LIBRARY ${OPENGL_LIBRARIES})

endif()

# This deprecated setting is for backward compatibility with CMake1.4
set(OPENGL_INCLUDE_PATH ${OPENGL_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set OPENGL_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenGL DEFAULT_MSG OPENGL_gl_LIBRARY)

mark_as_advanced(
  OPENGL_INCLUDE_DIR
  OPENGL_xmesa_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
)
