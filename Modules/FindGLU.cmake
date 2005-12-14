

# Use of this file is deprecated, and is here for backwards compatibility with CMake 1.4
# GLU library is now found by FindOpenGL.cmake
#
#

INCLUDE(FindOpenGL)

IF (OPENGL_GLU_FOUND)
  SET (GLU_LIBRARY ${OPENGL_LIBRARIES})
  SET (GLU_INCLUDE_PATH ${OPENGL_INCLUDE_DIR})
ENDIF (OPENGL_GLU_FOUND)

