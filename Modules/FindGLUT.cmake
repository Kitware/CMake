#
# try to find GLU on UNIX systems once done this will define
#
# GLUT_INCLUDE_PATH = where the GLUT include directory can be found
# GLUT_LIB_PATH     = where the GLUT library can be found
# GLUT_LIBRARY      = the name of the GLUT library to link with
#

IF (WIN32)
  FIND_LIBRARY(GLUT_LIBRARY glut32)

ELSE (WIN32)
  FIND_PATH(GLUT_INCLUDE_PATH GL/glut.h 
  /usr/include 
  /usr/include/GL
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )

  FIND_LIBRARY(GLUT_LIBRARY glut
  /usr/lib 
  /usr/local/lib 
  /opt/graphics/OpenGL/lib 
  /usr/openwin/lib 
  /usr/X11R6/lib
  )

ENDIF (WIN32)
