#
# try to find X11 on UNIX systems.
#
# The following values are defined
# X11_INCLUDE_DIR  - where to find X11.h
# X11_LIBRARIES    - link against these to use X11
# X11_FOUND        - True if X11 is available
# X11_Xext_FOUND   - True if the X11 extensions are available.

IF (UNIX)
  
  FIND_PATH(X11_INCLUDE_DIR X11/X.h
    /usr/include 
    /usr/local/include 
    /usr/openwin/include 
    /usr/openwin/share/include 
    /usr/X11R6/include 
    /usr/include/X11
    /opt/graphics/OpenGL/include
  )


  FIND_LIBRARY(X11_X11_LIBRARY X11
    /usr/lib 
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib
  )

  FIND_LIBRARY(X11_Xext_LIBRARY Xext
    /usr/lib 
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib
  )

  IF(X11_INCLUDE_DIR)

    IF(X11_X11_LIBRARY)
      SET( X11_FOUND "YES" )
      SET( X11_LIBRARIES ${X11_X11_LIBRARY} )
    ENDIF(X11_X11_LIBRARY)

    IF(X11_Xext_LIBRARY)
      SET( X11_LIBRARIES ${X11_LIBRARIES} ${X11_Xext_LIBRARY} )
      SET( X11_Xext_FOUND "YES")
    ENDIF(X11_Xext_LIBRARY)

  ENDIF(X11_INCLUDE_DIR)

  # Deprecated variable fro backwards compatibility with CMake 1.4
  SET (X11_LIBRARY ${X11_X11_LIBRARY})

MARK_AS_ADVANCED(
  X11_X11_LIBRARY
  X11_Xext_LIBRARY
  X11_INCLUDE_DIR
)

ENDIF (UNIX)
