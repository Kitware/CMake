# Try to find OpenGL
# Once done this will define
#
# OPENGL_FOUND        - system has OpenGL and it should be used
# OPENGL_XMESA_FOUND  - system has XMESA, and it should be used.
# OPENGL_GLU_FOUND    - system has GLU and it should be used.
# OPENGL_INCLUDE_DIR  - where the GL include directory can be found
# OPENGL_LIBRARIES    - Link these to use OpenGL
#
#
# Also defined, but not for general use are
# OPENGL_gl_LIBRARY   - Path to OpenGL Library
# OPENGL_glu_LIBRARY  - Pat to GLU Librar
#

IF (WIN32)
  IF (CYGWIN)


    FIND_PATH(OPENGL_INCLUDE_DIR GL/gl.h
      /usr/include
      /usr/X11R6/include
    )

    FIND_LIBRARY(OPENGL_gl_LIBRARY opengl32
      /usr/lib
      /usr/lib/w32api
    )

    FIND_LIBRARY(OPENGL_glu_LIBRARY glu32
      /usr/lib
      /usr/lib/w32api
    )

  ELSE (CYGWIN)

    IF(BORLAND)
      SET (OPENGL_gl_LIBRARY import32 CACHE STRING "OpenGL library for win32")
      SET (OPENGL_glu_LIBRARY import32 CACHE STRING "GLU library for win32")
    ELSE(BORLAND)
      SET (OPENGL_gl_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
      SET (OPENGL_glu_LIBRARY glu32 CACHE STRING "GLU library for win32")
    ENDIF(BORLAND)

    # No extra include path needed because OpenGL includes are with
    # the system includes but, cmake will create makefiles with
    # "-I${OPENGL_INCLUDE_DIR}" options if OPENGL_INCLUDE_DIR is
    # not set.  OPENGL_INCLUDE_DIR cannot be set to "" because the
    # resulting -I option to "cl" will eat the following
    # "-IC:\really\needed" option.  This is a kludge to get around
    # cmake not ignoring INCLUDE_DIRECTORIES commands with empty
    # strings.
    SET( OPENGL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}" )

  ENDIF (CYGWIN)

ELSE (WIN32)


  # The first line below is to make sure that the proper headers
  # are used on a Linux machine with the NVidia drivers installed.
  # They replace Mesa with NVidia's own library but normally do not
  # install headers and that causes the linking to
  # fail since the compiler finds the Mesa headers but NVidia's library.
  # Make sure the NVIDIA directory comes BEFORE the others.
  #  - Atanas Georgiev <atanas@cs.columbia.edu>

  FIND_PATH(OPENGL_INCLUDE_DIR GL/gl.h
    /usr/share/doc/NVIDIA_GLX-1.0/include
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include
    /usr/X11R6/include
  )

  FIND_PATH(OPENGL_xmesa_INCLUDE_DIR GL/xmesa.h
    /usr/share/doc/NVIDIA_GLX-1.0/include
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /opt/graphics/OpenGL/include
    /usr/X11R6/include
  )

  FIND_LIBRARY(OPENGL_gl_LIBRARY
    NAMES MesaGL GL
    PATHS /usr/lib
          /usr/local/lib
          /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/X11R6/lib
  )

  FIND_LIBRARY(OPENGL_glu_LIBRARY
    NAMES MesaGLU GLU
    PATHS /usr/lib
          /usr/local/lib
          /opt/graphics/OpenGL/lib
          /usr/openwin/lib
          /usr/X11R6/lib
  )



  ENDIF(OPENGL_glu_LIBRARY)
  ENDIF(OPENGL_gl_LIBRARY)
  ENDIF(OPENGL_INCLUDE_DIR)

ENDIF (WIN32)




SET( OPENGL_FOUND "NO" )
IF(OPENGL_INCLUDE_DIR)
  IF(OPENGL_gl_LIBRARY)

    IF(OPENGL_xmesa_INCLUDE_DIR)
      SET( OPENGL_XMESA_FOUND "YES" )
    ELSE(OPENGL_xmesa_INCLUDE_DIR)
      SET( OPENGL_XMESA_FOUND "NO" )
    ENDIF(OPENGL_xmesa_INCLUDE_DIR)

    IF(OPENGL_glu_LIBRARY)
      SET( OPENGL_GLU_FOUND "YES" )
    ENDIF(OPENGL_glu_LIBRARY)
      SET( OPENGL_GLU_FOUND "NO" )
    ENDIF(OPENGL_glu_LIBRARY)

    SET( OPENGL_FOUND "YES" )
    SET( OPENGL_LIBRARIES  ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_LIBRARIES})

#The following deprecated settings are for backwards compatibility with CMake1.4
    SET (OPENGL_LIBRARY ${OPENGL_LIBRARIES})
    SET (OPENGL_INCLUDE_PATH ${OPENGL_INCLUDE_DIR})

  ENDIF(OPENGL_gl_LIBRARY)
ENDIF(OPENGL_INCLUDE_DIR)
