#
# Try to find OpenGL, once done this will define:
#
# OPENGL_INCLUDE_PATH = where the GL include directory can be found
# OPENGL_LIBRARY      = the name of the OpenGL library to link with
# OPENGL_LIBRARY_PATH = where the GL library can be found (might be empty)
#

IF (WIN32)
  IF(BORLAND)
    SET (OPENGL_LIBRARY import32 CACHE STRING "OpenGL library for win32")
  ELSE(BORLAND)
    SET (OPENGL_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
  ENDIF(BORLAND)
ELSE (WIN32)
  IF (APPLE)
    SET(OPENGL_LIBRARY "-framework Carbon -framework AGL -framework OpenGL" CACHE STRING "OpenGL library for OSX")
  ELSE (APPLE)
     FIND_PATH(OPENGL_INCLUDE_PATH GL/gl.h 
       /usr/include 
       /usr/local/include 
       /usr/openwin/share/include 
       /opt/graphics/OpenGL/include 
       /usr/X11R6/include 
     )
     MARK_AS_ADVANCED(
       OPENGL_INCLUDE_PATH
     )

     FIND_LIBRARY(OPENGL_LIBRARY GL
       /usr/lib 
       /usr/local/lib 
       /opt/graphics/OpenGL/lib 
       /usr/openwin/lib 
       /usr/X11R6/lib
     )

   ENDIF (APPLE)
ENDIF (WIN32)

GET_FILENAME_COMPONENT (OPENGL_LIBRARY_PATH ${OPENGL_LIBRARY} PATH)

MARK_AS_ADVANCED(
  OPENGL_LIBRARY
  OPENGL_LIBRARY_PATH
)
