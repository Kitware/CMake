#
# try to find OpenGL on UNIX systems once done this will define
#
# OPENGL_INCLUDE_PATH = where the GL include directory can be found
# OPENGL_LIB_PATH     = where the GL library can be found
# OPENGL_LIBRARY      = the name of the OpenGL library to link with
#

IF (WIN32)
  SET (OPENGL_LIBRARY opengl32 CACHE)
ELSE (WIN32)
  
  FIND_PATH(OPENGL_INCLUDE_PATH GL/gl.h 
  /usr/include 
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )


  FIND_LIBRARY(OPENGL_LIB_PATH GL
  /usr/lib 
  /usr/local/lib 
  /opt/graphics/OpenGL/lib 
  /usr/openwin/lib 
  /usr/X11R6/lib
  )

  # right now we only look for -lgl maybe in future also mesa
  SET (OPENGL_LIBRARY GL CACHE)

ENDIF (WIN32)