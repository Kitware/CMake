#
# try to find OpenGL on UNIX systems once done this will define
#
# OPENGL_INCLUDE_PATH = where the GL include directory can be found
# OPENGL_LIB_PATH     = where the GL library can be found
# OPENGL_LIBRARY      = the name of the OpenGL library to link with
#

IF (WIN32)
  IF(BORLAND)
    SET (OPENGL_LIBRARY import32 CACHE STRING "OpenGL library for win32")
  ELSE(BORLAND)
    SET (OPENGL_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
  ENDIF(BORLAND)
ELSE (WIN32)
  FIND_PATH(OPENGL_INCLUDE_PATH GL/gl.h 
  /usr/include 
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )

  FIND_LIBRARY(OPENGL_LIBRARY GL
  /usr/lib 
  /usr/local/lib 
  /opt/graphics/OpenGL/lib 
  /usr/openwin/lib 
  /usr/X11R6/lib
  )

ENDIF (WIN32)