# Try to find Mesa off-screen library and include dir.
# Once done this will define
#
# OSMESA_INCLUDE_DIR  - where the GL/osmesa.h can be found
# OSMESA_LIBRARY      - Link this to use OSMesa


IF (NOT OSMESA_INCLUDE_DIR)
  FIND_PATH(OSMESA_INCLUDE_DIR GL/osmesa.h 
    /usr/include 
    /usr/local/include 
    /usr/openwin/share/include 
    /opt/graphics/OpenGL/include 
    /usr/X11R6/include 
  )
ENDIF (NOT OSMESA_INCLUDE_DIR)

# This may be left blank if OSMesa symbols are included
# in the main Mesa library
IF (NOT OSMESA_LIBRARY)
  FIND_LIBRARY(OSMESA_LIBRARY OSMesa
    /usr/lib 
    /usr/local/lib 
    /opt/graphics/OpenGL/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib
  )
ENDIF (NOT OSMESA_LIBRARY)

