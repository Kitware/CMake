#
# Try to find GLUT, once done this will define:
#
# GLUT_INCLUDE_PATH = where the GLUT include directory can be found
# GLUT_LIBRARY      = the name of the GLUT library to link with
#

IF (WIN32)
  IF (CYGWIN)
    FIND_LIBRARY(GLUT_LIBRARY glut32
      ${OPENGL_LIBRARY_PATH}
      /usr/lib/w32api
      /usr/lib 
      /usr/local/lib 
      /usr/X11R6/lib
    )
  ELSE (CYGWIN)
    FIND_LIBRARY(GLUT_LIBRARY glut32
      ${OPENGL_LIBRARY_PATH}
    )
  ENDIF (CYGWIN)
ELSE (WIN32)
  IF (APPLE)
    SET(GLUT_LIBRARY "-framework Glut" CACHE STRING "GLUT library for OSX")
  ELSE (APPLE)
    FIND_PATH(GLUT_INCLUDE_PATH GL/glut.h 
      ${OPENGL_INCLUDE_PATH}
      /usr/include 
      /usr/include/GL
      /usr/local/include 
      /usr/openwin/share/include 
      /opt/graphics/OpenGL/include 
      /usr/X11R6/include 
    )
    MARK_AS_ADVANCED(
      GLUT_INCLUDE_PATH
    )

    FIND_LIBRARY(GLUT_LIBRARY glut
      ${OPENGL_LIBRARY_PATH}
      /usr/lib 
      /usr/local/lib 
      /opt/graphics/OpenGL/lib 
      /usr/openwin/lib 
      /usr/X11R6/lib
    )

   ENDIF (APPLE)
ENDIF (WIN32)

MARK_AS_ADVANCED(
  GLUT_LIBRARY
)
