#
# try to find GLU on UNIX systems once done this will define
#
# GLU_INCLUDE_PATH = where the GLU include directory can be found
# GLU_LIB_PATH     = where the GLU library can be found
# GLU_LIBRARY      = the name of the GLU library to link with
#

IF (WIN32)
  IF(BORLAND)
    SET (GLU_LIBRARY import32 CACHE STRING "GLU library for win32")
  ELSE(BORLAND)
    SET (GLU_LIBRARY glu32 CACHE STRING "GLU library for win32")
  ENDIF(BORLAND)
ELSE (WIN32)
  FIND_PATH(GLU_INCLUDE_PATH GL/glu.h 
  /usr/include 
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )
  MARK_AS_ADVANCED(
    GLU_INCLUDE_PATH
  )

  FIND_LIBRARY(GLU_LIBRARY GLU
  /usr/lib 
  /usr/local/lib 
  /opt/graphics/OpenGL/lib 
  /usr/openwin/lib 
  /usr/X11R6/lib
  )

ENDIF (WIN32)

MARK_AS_ADVANCED(
  GLU_LIBRARY
)
