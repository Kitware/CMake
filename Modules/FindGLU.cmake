#
# Try to find GLU, once done this will define:
#
# GLU_INCLUDE_PATH = where the GLU include directory can be found
# GLU_LIBRARY      = the name of the GLU library to link with
#

IF (WIN32)
  IF(BORLAND)
    SET (GLU_LIBRARY import32 CACHE STRING "GLU library for win32")
  ELSE(BORLAND)
    SET (GLU_LIBRARY glu32 CACHE STRING "GLU library for win32")
  ENDIF(BORLAND)
ELSE (WIN32)
  IF (APPLE)
    # The GLU lib is included in the OpenGL framework. We can not 
    # set GLU_LIBRARY to an empty string since it is equal to FALSE if
    # this variable is used in a IF test. So let's set it to the OpenGL
    # framework. It does not harm to duplicate the OpenGL framework and it
    # might even help in case OPENGL_LIBRARY is not set.
    SET(GLU_LIBRARY "-framework AGL -framework OpenGL" CACHE STRING "GLU library for OSX")
  ELSE (APPLE)
    FIND_PATH(GLU_INCLUDE_PATH GL/glu.h
      ${OPENGL_INCLUDE_PATH}
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
  GLU_LIBRARY
)
