#
# try to find X11 on UNIX systems.
#
# The following values are defined
# CMAKE_X11_INCLUDE_PATH  - where to find X11.h
# CMAKE_X_LIBS            - link against these to use X11
# CMAKE_HAS_X             - True if X11 is available
# CMAKE_X11_LIBDIR        - Directory with X11 library
# CMAKE_Xext_LIBDIR       - Directory with Xext library

IF (UNIX)
  SET(CMAKE_X_PRE_LIBS "")
  SET(CMAKE_X_REAL_LIBS "")
  SET(CMAKE_X_EXTRA_LIBS "")
  SET(CMAKE_HAS_X 0)
  
  FIND_PATH(CMAKE_X11_INCLUDE_PATH X11/X.h
    /usr/include 
    /usr/local/include 
    /usr/openwin/include 
    /usr/openwin/share/include 
    /usr/X11R6/include 
    /usr/include/X11
    /opt/graphics/OpenGL/include
  )

  FIND_PATH(CMAKE_Xlib_INCLUDE_PATH X11/Xlib.h
    /usr/include 
    /usr/local/include 
    /usr/openwin/include 
    /usr/openwin/share/include 
    /usr/X11R6/include 
    /usr/include/X11
    /opt/graphics/OpenGL/include
  )

  FIND_PATH(CMAKE_Xutil_INCLUDE_PATH X11/Xutil.h
    /usr/include 
    /usr/local/include 
    /usr/openwin/include 
    /usr/openwin/share/include 
    /usr/X11R6/include 
    /usr/include/X11
    /opt/graphics/OpenGL/include
  )

  IF(CMAKE_X11_INCLUDE_PATH)
    SET(CMAKE_X11_INCLUDES "${CMAKE_X11_INCLUDES};${CMAKE_X11_INCLUDE_PATH}")
  ENDIF(CMAKE_X11_INCLUDE_PATH)
  IF(CMAKE_Xlib_INCLUDE_PATH)
    SET(CMAKE_X11_INCLUDES "${CMAKE_X11_INCLUDES};${CMAKE_Xlib_INCLUDE_PATH}")
  ENDIF(CMAKE_Xlib_INCLUDE_PATH)
  IF(CMAKE_Xutil_INCLUDE_PATH)
    SET(CMAKE_X11_INCLUDES "${CMAKE_X11_INCLUDES};${CMAKE_Xutil_INCLUDE_PATH}")
  ENDIF(CMAKE_Xutil_INCLUDE_PATH)

  FIND_LIBRARY(CMAKE_X11_LIBDIR X11
    /usr/lib 
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib
  )

  FIND_LIBRARY(CMAKE_Xext_LIBDIR Xext
    /usr/lib 
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib
  )

  IF(CMAKE_X11_INCLUDE_PATH)

    IF(CMAKE_X11_LIBDIR)
      SET( CMAKE_X_LIBS "${CMAKE_X_LIBS};${CMAKE_X11_LIBDIR}" )
    ENDIF(CMAKE_X11_LIBDIR)

    IF(CMAKE_Xext_LIBDIR)
      SET( CMAKE_X_LIBS "${CMAKE_X_LIBS};${CMAKE_Xext_LIBDIR}" )
    ENDIF(CMAKE_Xext_LIBDIR)

  ENDIF(CMAKE_X11_INCLUDE_PATH)

  # Deprecated variable fro backwards compatibility with CMake 1.4
  IF(CMAKE_X11_INCLUDE_PATH)
    IF(CMAKE_X_LIBS)
      SET(CMAKE_HAS_X 1)
    ENDIF(CMAKE_X_LIBS)
  ENDIF(CMAKE_X11_INCLUDE_PATH)
  IF(CMAKE_HAS_X)
    SET (CMAKE_X_LIBS "${CMAKE_X_PRE_LIBS};${CMAKE_X_LIBS};${CMAKE_X_EXTRA_LIBS}" 
         CACHE STRING 
         "Libraries and options used in X11 programs.")

    SET (CMAKE_X_CFLAGS           "${CMAKE_X_CFLAGS}" CACHE STRING 
         "X11 extra flags.")
  ENDIF(CMAKE_HAS_X)
  SET (CMAKE_HAS_X ${CMAKE_HAS_X} CACHE INTERNAL "Is X11 around.")
  MARK_AS_ADVANCED(
    CMAKE_X11_INCLUDE_PATH
    CMAKE_X11_LIBDIR
    CMAKE_Xext_LIBDIR
    CMAKE_Xlib_INCLUDE_PATH
    CMAKE_Xutil_INCLUDE_PATH
    )
ENDIF (UNIX)
