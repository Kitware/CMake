# - try to find glut library and include files
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
# Also defined, but not for general use are:
#  GLUT_glut_LIBRARY = the full path to the glut library.
#  GLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  GLUT_Xi_LIBRARY   = the full path to the Xi Library.

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

if(WIN32)
  find_path( GLUT_INCLUDE_DIR NAMES GL/glut.h
    PATHS  ${GLUT_ROOT_PATH}/include )
  find_library( GLUT_glut_LIBRARY NAMES glut glut32 freeglut
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${GLUT_ROOT_PATH}/Release
    )
else(WIN32)

  if(APPLE)
    # These values for Apple could probably do with improvement.
    find_path( GLUT_INCLUDE_DIR glut.h
      /System/Library/Frameworks/GLUT.framework/Versions/A/Headers
      ${OPENGL_LIBRARY_DIR}
      )
    set(GLUT_glut_LIBRARY "-framework GLUT" CACHE STRING "GLUT library for OSX")
    set(GLUT_cocoa_LIBRARY "-framework Cocoa" CACHE STRING "Cocoa framework for OSX")
  else(APPLE)

    find_path( GLUT_INCLUDE_DIR GL/glut.h
      /usr/include/GL
      /usr/openwin/share/include
      /usr/openwin/include
      /opt/graphics/OpenGL/include
      /opt/graphics/OpenGL/contrib/libglut
      )

    find_library( GLUT_glut_LIBRARY glut
      /usr/openwin/lib
      )

    find_library( GLUT_Xi_LIBRARY Xi
      /usr/openwin/lib
      )

    find_library( GLUT_Xmu_LIBRARY Xmu
      /usr/openwin/lib
      )

  endif(APPLE)

endif(WIN32)

set( GLUT_FOUND "NO" )
if(GLUT_INCLUDE_DIR)
  if(GLUT_glut_LIBRARY)
    # Is -lXi and -lXmu required on all platforms that have it?
    # If not, we need some way to figure out what platform we are on.
    set( GLUT_LIBRARIES
      ${GLUT_glut_LIBRARY}
      ${GLUT_Xmu_LIBRARY}
      ${GLUT_Xi_LIBRARY}
      ${GLUT_cocoa_LIBRARY}
      )
    set( GLUT_FOUND "YES" )

    #The following deprecated settings are for backwards compatibility with CMake1.4
    set(GLUT_LIBRARY ${GLUT_LIBRARIES})
    set(GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})

  endif(GLUT_glut_LIBRARY)
endif(GLUT_INCLUDE_DIR)

mark_as_advanced(
  GLUT_INCLUDE_DIR
  GLUT_glut_LIBRARY
  GLUT_Xmu_LIBRARY
  GLUT_Xi_LIBRARY
  )
