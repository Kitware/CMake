#
# try to find X11 libraries on UNIX systems once done this will define
#
# X11_INCLUDE_PATH = where the GL include directory can be found
# X11_LIB_PATH     = where the GL library can be found
# X11_LIBRARY      = the name of the OpenGL library to link with
#

IF (WIN32)
  SET (X11_LIBRARY x11 CACHE STRING "X11 library")
ELSE (WIN32)
  FIND_PATH(X11_INCLUDE_PATH X11.h
  /usr/include 
  /usr/local/include 
  /usr/openwin/share/include 
  /opt/graphics/OpenGL/include 
  /usr/X11R6/include 
  )

  FIND_LIBRARY(X11_LIBRARY libX11.so
  /usr/lib 
  /usr/local/lib 
  /usr/X11R6/lib
  /usr/openwin/lib 
  )

ENDIF (WIN32)
