#
# try to find GLU on UNIX systems once done this will define
#
# GLU_INCLUDE_PATH = where the GLU include directory can be found
# GLU_LIB_PATH     = where the GLU library can be found
# GLU_LIBRARY      = the name of the GLU library to link with
#

IF (WIN32)
  SET (GLU_LIBRARY glu32 CACHE STRING "GLU library for win32")
ELSE (WIN32)
  FIND_PATH(GLU_INCLUDE_PATH GL/gl.h 
  /usr/include 
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )

  FIND_LIBRARY(GLU_LIBRARY GLU
  /usr/lib 
  /usr/local/lib 
  /opt/graphics/OpenGL/lib 
  /usr/openwin/lib 
  /usr/X11R6/lib
  )

ENDIF (WIN32)
